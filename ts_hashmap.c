#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ts_hashmap.h"

/**
 * Creates a new thread-safe hashmap. 
 *
 * @param capacity initial capacity of the hashmap.
 * @return a pointer to a new thread-safe hashmap.
 */
ts_hashmap_t *initmap(int capacity) {
//initialize a hashmap sturct and then give it a table of values
  ts_hashmap_t* map=(ts_hashmap_t*) malloc(sizeof(ts_hashmap_t));
  ts_entry_t** xtable=(ts_entry_t**) malloc(capacity * sizeof(ts_entry_t*));
  map->table=xtable;
  map->capacity=capacity;
  map->size=0;

//initilize individual locks for each bin and one extra lock for size;
  pthread_mutex_t** locksarr=(pthread_mutex_t**) malloc(capacity*sizeof(pthread_mutex_t*));
  for(int i=0; i<capacity;i++){
    locksarr[i]=(pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(locksarr[i],NULL);
  }

  pthread_mutex_t* biggie=(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
  map->sizelock=biggie;
  map->locks=locksarr;

  return map;
}

/**
 * Obtains the value associated with the given key.
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int get(ts_hashmap_t *map, int key) {
//get the location where the value will be stored;
  int loc=((unsigned int)key)%map->capacity;
  if(map->table[loc]!=NULL){

    //if its not the first item then begin searching
  if(map->table[loc]->key!=key){
    ts_entry_t *temp=map->table[loc]->next;
    if(temp==NULL){
      return INT_MAX;
    }
    while(temp->key!=key){
      temp=temp->next;
      if(temp==NULL){
        return INT_MAX;
      }
    }

    return temp->value;
  }
  return map->table[loc]->value;
  }
  return INT_MAX;
}

/**
 * Associates a value associated with a given key.
 * @param map a pointer to the map
 * @param key a key
 * @param value a value
 * @return old associated value, or INT_MAX if the key was new
 */
int put(ts_hashmap_t *map, int key, int value) {
//get the location where the value will be stored;
  int loc=((unsigned int)key)%map->capacity;
    if(map->table[loc]!=NULL){
        //check if it is the first item if not begin searching
        if(map->table[loc]->key!=key){
            ts_entry_t *temp=map->table[loc]->next;
            if(temp==NULL){
                ts_entry_t *node=(ts_entry_t*) malloc(sizeof(ts_entry_t));
                node->key=key;
                node->value=value;

                //once found key and value can be set freely however a lock must be made when seting next and adding the pair to the table
                pthread_mutex_lock(map->locks[loc]);
                node->next=map->table[loc];
                map->table[loc]=node;
                pthread_mutex_unlock(map->locks[loc]);

                //also a lock must be added when incrementing size;
                pthread_mutex_lock(map->sizelock);
                map->size++;
                pthread_mutex_unlock(map->sizelock);
                return INT_MAX;
            }
            while(temp->key!=key){
                temp=temp->next;
                if(temp==NULL){
                    ts_entry_t *node=(ts_entry_t*) malloc(sizeof(ts_entry_t));
                    node->key=key;
                    node->value=value;

                     //once found key and value can be set freely however a lock must be made when seting next and adding the pair to the table
                    pthread_mutex_lock(map->locks[loc]);
                    node->next=map->table[loc];
                    map->table[loc]=node;
                    pthread_mutex_unlock(map->locks[loc]);

                    pthread_mutex_lock(map->sizelock);
                    map->size++;
                    pthread_mutex_unlock(map->sizelock);
                    return INT_MAX;
                }
            }
             //once found key and value can be set freely however a lock must be made when seting next and adding the pair to the table
            pthread_mutex_lock(map->locks[loc]);
            int val=temp->value;
            temp->value=value;
            pthread_mutex_unlock(map->locks[loc]);
            return val;
        }
         //once found key and value can be set freely however a lock must be made when seting next and adding the pair to the table
        pthread_mutex_lock(map->locks[loc]);
        int val=map->table[loc]->value;
        map->table[loc]->value=value;
        pthread_mutex_unlock(map->locks[loc]);
        return val;
      }
    else{
        ts_entry_t *node=(ts_entry_t*) malloc(sizeof(ts_entry_t));
        node->key=key;
        node->value=value;
        node->next=NULL;
        //once found key and value can be set freely however a lock must be made when adding the pair to the table
        pthread_mutex_lock(map->locks[loc]);
        map->table[loc]=node;
        pthread_mutex_unlock(map->locks[loc]);

        pthread_mutex_lock(map->sizelock);
        map->size++;
        pthread_mutex_unlock(map->sizelock);
    }
  return INT_MAX;
}

/**
 * Removes an entry in the map
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int del(ts_hashmap_t *map, int key) {
  int loc=((unsigned int)key)%map->capacity;
  if(map->table[loc]!=NULL){
   
  if(map->table[loc]->key!=key){
    ts_entry_t *prev=map->table[loc];
    ts_entry_t *temp=map->table[loc]->next;
    if(temp==NULL){
      return INT_MAX;
    }
    while(temp->key!=key){
      prev=temp;
      temp=temp->next;
      if(temp==NULL){
        return INT_MAX;
      }
    }
    pthread_mutex_lock(map->locks[loc]);
    prev->next=temp->next;
    pthread_mutex_unlock(map->locks[loc]);
    pthread_mutex_lock(map->sizelock);
    map->size--;
    pthread_mutex_unlock(map->sizelock);
    return temp->value;
  }
    pthread_mutex_lock(map->locks[loc]);
    int val=map->table[loc]->value;
    map->table[loc]=map->table[loc]->next;
    pthread_mutex_unlock(map->locks[loc]);
    pthread_mutex_lock(map->sizelock);
    map->size--;
    pthread_mutex_unlock(map->sizelock);
     return val;
  
  }
  return INT_MAX;
}


/**
 * @return the load factor of the given map
 */
double lf(ts_hashmap_t *map) {
  return (double) map->size / map->capacity;
}

/**
 * Prints the contents of the map
 */
void printmap(ts_hashmap_t *map) {
  for (int i = 0; i < map->capacity; i++) {
    printf("[%d] -> ", i);
    ts_entry_t *entry = map->table[i];
    while (entry != NULL) {
      printf("(%d,%d)", entry->key, entry->value);
      if (entry->next != NULL)
        printf(" -> ");
      entry = entry->next;
    }
    printf("\n");
  }
}

int assert_equals(ts_hashmap_t *map1, ts_hashmap_t *map2){
    for(int i=0; i<map1->capacity; i++){
        if(map1->table[i]!=NULL){
        if(map1->table[i]->value==map2->table[i]->value){
            if(map1->table[i]->key==map2->table[i]->key){
                ts_entry_t* temp1=map1->table[i]->next;
                ts_entry_t* temp2=map2->table[i]->next;
                while(temp1!=NULL){
                    if(temp1->value==temp2->value){
                        if(temp1->key==temp2->key){
                            temp1=temp1->next;
                            temp2=temp2->next;
                        }
                        else{
                            printf("here 1");
                            return 0;
                        }
                    }
                    else{
                         printf("here 2");
                        return 0;
                    }
                }
            }
            else{
                 printf("here 3");
                return 0;
            }
           
        }
        else{
             printf("here 4");
                return 0;
        }
        }
    }
    return 1;
}