#include "hash_lock.h"

#include <stdio.h>
#include <stdlib.h>

void hashInit(hash_lock_t* bucket) {
  int i;
  for(i=0;i<HASHNUM;i++) {
    pthread_mutex_init(&bucket->table[i].mutex,NULL);
  }
}

int getValue(hash_lock_t* bucket, int key) {
  int i=HASH(key);
  int found=-1;
  pthread_mutex_lock(&bucket->table[i].mutex);
  Hlist cur=bucket->table[i].head;
  while(cur!=NULL) {
    if(cur->key==key) {
      found=cur->value;
    }
    cur=cur->next;
  }
  pthread_mutex_unlock(&bucket->table[i].mutex);
  return found;
}

void insert(hash_lock_t* bucket, int key, int value) {
  int i=HASH(key);
  pthread_mutex_lock(&bucket->table[i].mutex);
  Hlist cur=bucket->table[i].head;
  while(cur!=NULL) {
    if(cur->key==key) {
      cur->value=value;
      pthread_mutex_unlock(&bucket->table[i].mutex);
      return;
    } else {
      cur=cur->next;
    }
  }

  Hlist node=calloc(1,sizeof(Hnode));
  node->key=key;
  node->value=value;
  node->next=NULL;

  node->next=bucket->table[i].head;
  bucket->table[i].head=node;

  pthread_mutex_unlock(&bucket->table[i].mutex);
}

int setKey(hash_lock_t* bucket, int key, int new_key) {
  int i=HASH(key);
  pthread_mutex_lock(&bucket->table[i].mutex);
  Hlist cur=bucket->table[i].head;
  if(cur==NULL) {
    pthread_mutex_unlock(&bucket->table[i].mutex);
    return -1;
  }
  int found=0;
  Hlist temp=cur;
  if(bucket->table[i].head->key==key) {
    bucket->table[i].head=cur->next;
    found=1;
  } else {
    while(cur!=NULL) {
      if(cur->key==key) {
        found=1;
        temp->next=cur->next;
        break;
      } else {
        temp=cur;
        cur=cur->next;
      }
    }
  }
  int value;
  if(found==1) {
    value=cur->value;
    free(cur);
  }
  pthread_mutex_unlock(&bucket->table[i].mutex);
  if(found==0) return -1;

  int k=HASH(new_key);
  pthread_mutex_lock(&bucket->table[k].mutex);

  Hlist t=bucket->table[k].head;
  Hlist node=calloc(1,sizeof(Hnode));
  node->key=new_key;
  node->value=value;

  // node->next=bucket->table[k].head;
  // bucket->table[k].head=node;

  if(t==NULL) {
    bucket->table[k].head=node;
  } else {
    while(t->next!=NULL) {
      t=t->next;
    }
    t->next=node;
  }
  pthread_mutex_unlock(&bucket->table[k].mutex);
  return 0;
}