#include "list_lock.h"

#include <stdio.h>
#include <stdlib.h>

void listInit(list_lock_t* list) {
  list->head=NULL;
  pthread_mutex_init(&list->mutex,NULL);
  pthread_cond_init(&list->cond,NULL);
}

void producer(list_lock_t* list, DataType value) {
  pthread_mutex_lock(&list->mutex);

  struct node *node=calloc(1,sizeof(struct node));
  node->value=value;
  node->next=NULL;
  if(list->head==NULL) {
    list->head=node;
  } else {
    struct node *cur=list->head;
    while(cur->next!=NULL) {
      cur=cur->next;
    }
    cur->next=node;
  }
  pthread_cond_signal(&list->cond);
  pthread_mutex_unlock(&list->mutex);
}

void consumer(list_lock_t* list) {
  pthread_mutex_lock(&list->mutex);
  while(list->head==NULL) {
    pthread_cond_wait(&list->cond,&list->mutex);
  }
  struct node *cur=list->head;
  list->head=cur->next;
  free(cur);
  pthread_mutex_unlock(&list->mutex);
}

int getListSize(list_lock_t* list) {
  pthread_mutex_lock(&list->mutex);
  int count=0;
  struct node *cur=list->head;
  while(cur!=NULL) {
    cur=cur->next;
    count++;
  }
  pthread_mutex_unlock(&list->mutex);
  return count;
}