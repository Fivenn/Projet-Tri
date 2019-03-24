/**
 * @file
 * @brief Implementation of the V3 of the system project.
 * @warning You should not modifie the current file.
 */

#include "project_v3.h"
#include <pthread.h>

/**
 * @brief Maximum length (in character) for a file name.
 **/
#define PROJECT_FILENAME_MAX_SIZE 1024

/**
 * @brief Type of the sort algorithm used in this version.
 **/
//#define SORTALGO(nb_elem, values) SU_ISort(nb_elem, values)
//#define SORTALGO(nb_elem, values) SU_HSort(nb_elem, values)
#define SORTALGO(nb_elem, values) SU_QSort(nb_elem, values)

/**********************************/

void projectV3(const char * i_file, const char * o_file, unsigned long nb_split){

  /* Get number of line to sort */
  int nb_print = 0;
  unsigned long nb_lines = SU_getFileNbLine(i_file);
  unsigned long nb_lines_per_files = nb_lines/ (unsigned long) nb_split;
  fprintf(stderr,
	  "Projet1 version with %lu split of %lu lines\n",
	  nb_split,
	  nb_lines_per_files);

  /* 0 - Deal with case nb_split = 1 */
  if(nb_split < 2){
    int * values = NULL;
    unsigned long nb_elem = SU_loadFile(i_file, &values);

    SORTALGO(nb_elem, values);

    SU_saveFile(o_file, nb_elem, values);

    free(values);
    return;
  }

  /* 1 - Spit the source file */

  /* 1.1 - Create a vector of target filenames for the split */
  char ** filenames = (char**) malloc(sizeof(char*) * (size_t) nb_split);
  char ** filenames_sort = (char**) malloc(sizeof(char*) * (size_t) nb_split);
  unsigned long cpt = 0;
  for(cpt = 0; cpt < nb_split; ++cpt){
    filenames[cpt] = (char *) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
    nb_print = snprintf(filenames[cpt],
			PROJECT_FILENAME_MAX_SIZE,
			"/tmp/tmp_split_%d_%lu.txt",getpid(), cpt);
    if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
      err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
    }

    filenames_sort[cpt] = (char *) malloc(sizeof(char) * PROJECT_FILENAME_MAX_SIZE);
    nb_print = snprintf(filenames_sort[cpt],
			PROJECT_FILENAME_MAX_SIZE,
			"/tmp/tmp_split_%d_%lu.sort.txt",getpid(), cpt);
    if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
      err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
    }
  }

  /* 1.2 - Split the source file */
  SU_splitFile2(i_file,
		nb_lines_per_files,
		nb_split,
		(const char **) filenames
		);

  /* 2 - Sort each file */
  projectV3_sortFiles(nb_split, (const char **) filenames, (const char **) filenames_sort);

  /* 3 - Merge (two by two) */
  projectV3_combMerge(nb_split, (const char **) filenames_sort, (const char *) o_file);

  /* 4 - Clear */
  for(cpt = 0; cpt < nb_split; ++cpt){
    free(filenames[cpt]); // not needed :  clear in sort
    free(filenames_sort[cpt]);
  }

  free(filenames);
  free(filenames_sort);

}

void *threadSort(void *arg){
  T_InfoThread *a = arg;

  fprintf(stderr, "Inner sort %lu: Array of %lu elem\n", a->cpt, a->nb_elem);
  SORTALGO(a->nb_elem, a->values);
  SU_saveFile(a->filenames_sort[a->cpt], a->nb_elem, a->values);
  free(a->values);

  pthread_exit(NULL);
}

void projectV3_sortFiles(unsigned long nb_split, const char ** filenames, const char ** filenames_sort){
  unsigned long cpt = 0;
	pthread_t* thread = (pthread_t*)malloc((long unsigned int)nb_split * sizeof(pthread_t));

    for(cpt = 0; cpt < nb_split; ++cpt){
      T_InfoThread *infoT = (T_InfoThread*)malloc(sizeof(T_InfoThread));

      int * values = NULL;
      unsigned long nb_elem = SU_loadFile(filenames[cpt], &values);
      SU_removeFile(filenames[cpt]);

			infoT->nb_elem = nb_elem;
			infoT->cpt = cpt;
			infoT->values = values;
      infoT->filenames = filenames;
	    infoT->filenames_sort = filenames_sort;

      if (pthread_create(&thread[cpt], NULL, threadSort,infoT) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
      }
    }

    for(cpt = 0; cpt < nb_split; ++cpt) {
      if (pthread_join(thread[cpt], NULL) != 0) {
        perror("pthread_join");
        exit(EXIT_FAILURE);
      }
    }
    
}

void projectV3_combMerge(unsigned long nb_split, const char ** filenames_sort, const char * o_file){

  pthread_t thread1, thread2;
  char *tmp_file1 = "/tmp/tmpfile_split_merge_1.txt", *tmp_file2 = "/tmp/tmpfile_split_merge_2.txt";
  T_InfoThreadMerge *infoT1 = (T_InfoThreadMerge*)malloc(sizeof(T_InfoThreadMerge));
  T_InfoThreadMerge *infoT2 = (T_InfoThreadMerge*)malloc(sizeof(T_InfoThreadMerge));

  infoT1->begin_index = 1;
  infoT1->end_index = nb_split/2;
  infoT1->filenames_sort = filenames_sort;
  infoT1->o_file = tmp_file1;

  infoT2->begin_index = (nb_split/2)+1;
  infoT2->end_index = nb_split;
  infoT2->filenames_sort = filenames_sort;
  infoT2->o_file = tmp_file2;

  if (pthread_create(&thread1, NULL, threadMerge, infoT1) != 0) {
    perror("pthread_create");
    exit(EXIT_FAILURE);
  }
  if (pthread_create(&thread2, NULL, threadMerge, infoT2) != 0) {
    perror("pthread_create");
    exit(EXIT_FAILURE);
  }

  if (pthread_join(thread1, NULL) != 0) {
    perror("pthread_join");
    exit(EXIT_FAILURE);
  }
  if (pthread_join(thread2, NULL) != 0) {
    perror("pthread_join");
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "Last merge sort : %s + %s -> %s \n",
	  tmp_file1,
	  tmp_file2,
	  o_file);
  SU_mergeSortedFiles(tmp_file1,
		      tmp_file2,
		      o_file);
  SU_removeFile(tmp_file1);
  SU_removeFile(tmp_file2);
}

void *threadMerge(void *arg) {
  T_InfoThreadMerge *a = arg;
  unsigned long cpt = 0;
  int nb_print = 0;
  
  char previous_name [PROJECT_FILENAME_MAX_SIZE];
  nb_print = snprintf(previous_name,
		      PROJECT_FILENAME_MAX_SIZE,
		      "%s", a->filenames_sort[a->begin_index-1]);
  if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
    err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
  }

  char current_name [PROJECT_FILENAME_MAX_SIZE];
  nb_print = snprintf(current_name,
		      PROJECT_FILENAME_MAX_SIZE,
		      "/tmp/tmp_split_%d_merge_%ld.txt", getpid(), a->begin_index-1);
  if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
    err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
  }

  for(cpt = a->begin_index; cpt < a->end_index - 1; ++cpt){
    printf("filename_sort : %s\n previous_name : %s\n, current_name : %s\n\n", a->filenames_sort[cpt], previous_name, current_name);
    fprintf(stderr, "Merge thread sort %lu : %s + %s -> %s \n",
	    cpt,
	    previous_name,
	    a->filenames_sort[cpt],
	    current_name);
    SU_mergeSortedFiles(previous_name,
			a->filenames_sort[cpt],
			current_name);
    SU_removeFile(previous_name);
    SU_removeFile(a->filenames_sort[cpt]);

    nb_print = snprintf(previous_name,
			PROJECT_FILENAME_MAX_SIZE,
			"%s", current_name);
    if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
      err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
    }

    nb_print = snprintf(current_name,
			PROJECT_FILENAME_MAX_SIZE,
			"/tmp/tmp_split_%d_merge_%lu.txt",getpid(), cpt);
    if(nb_print >= PROJECT_FILENAME_MAX_SIZE){
      err(1, "Out of buffer (%s:%d)", __FILE__, __LINE__ );
    }
  }

  /* Last merge */
  fprintf(stderr, "Last merge thread sort : %s + %s -> %s \n",
	  previous_name,
	  a->filenames_sort[a->end_index-1],
	  a->o_file);
  SU_mergeSortedFiles(previous_name,
		      a->filenames_sort[a->end_index-1],
		      a->o_file);
  SU_removeFile(previous_name);
  SU_removeFile(a->filenames_sort[a->end_index-1]);

  pthread_exit(NULL);
}