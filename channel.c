#include "channel.h"

// Creates a new channel with the provided size and returns it to the caller
// A 0 size indicates an unbuffered channel, whereas a positive size indicates a buffered channel
channel_t* channel_create(size_t size)
{
  /*In Comments*/
  /*Unbuffered Channel*/
  if(size == 0){
    channel_t* channel = (channel_t*)malloc(sizeof(channel_t));
        channel->data  = (void**)malloc(size * sizeof(void*));
        channel->buffer = NULL;
        channel->isOpen = 0;/* to check if channel is free?*/
        channel->bufferCapacity = 0;/*To know the original capacity of the buffer*/
        pthread_mutex_init(&(channel->mutex), NULL);//Initializeing pthread mutex
        pthread_cond_init(&(channel->RV),NULL);
        pthread_cond_init(&(channel->SD),NULL);
        return channel;
  }
  /*BUffered Channel*/
else if(size > 0){
    /* IMPLEMENT THIS */
      /*Malloc Channel as per the slide*/
        channel_t* channel = (channel_t*)malloc(sizeof(channel_t));
        channel->data  = (void**)malloc(size * sizeof(void*));
        channel->buffer = buffer_create(size);
        channel->isOpen = 0;/* to check if channel is free?*/
        channel->bufferCapacity = 0;/*To know the original capacity of the buffer*/
        pthread_mutex_init(&(channel->mutex), NULL);//Initializeing pthread mutex
        pthread_cond_init(&(channel->RV),NULL);
        pthread_cond_init(&(channel->SD),NULL);
        return channel;
}
else{
    return NULL;
}
}
// Writes data to the given channel
// This is a blocking call i.e., the function only returns on a successful completion of send
// In case the channel is full, the function waits till the channel has space to write the new data
// Returns SUCCESS for successfully writing data to the channel,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_send(channel_t *channel, void* data)
{
    /* IMPLEMENT THIS */
	pthread_mutex_lock(&channel->mutex);
    //channel is closed
    if(channel->isOpen == 1){
       pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
    /*If channel is Open*/
      /*Blocking other channels*/
      //size_t capacity = channel->buffer->size;
      /*hile(channel->buffer->size <= channel->buffer->capacity){
          pthread_cond_wait(&channel->SD,&channel->mutex); 
          buffer_add(channel->buffer,data);
          //pthread_cond_wait(&channel->SD,&channel->mutex); 
         // pthread_cond_signal(&channel->RV);
      }  */
      size_t capacity = channel->buffer->size;
      while(capacity != channel->buffer->capacity){
          pthread_cond_wait(&channel->SD,&channel->mutex); 
          //buffer_add(channel->buffer,data);
      // pthread_cond_signal(&channel->RV);
         pthread_mutex_unlock(&channel->mutex);
      }  
        buffer_add(channel->buffer,data);
        capacity = channel->buffer->size;
        pthread_cond_signal(&channel->RV);
        pthread_mutex_unlock(&channel->mutex);
        return SUCCESS;
    
}


// Reads data from the given channel and stores it in the function’s input parameter, data (Note that it is a double pointer).
// This is a blocking call i.e., the function only returns on a successful completion of receive
// In case the channel is empty, the function waits till the channel has some data to read
// Returns SUCCESS for successful retrieval of data,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_receive(channel_t* channel, void** data)
{
   /* IMPLEMENT THIS */
    //channel is closed
    pthread_mutex_lock(&channel->mutex); 
     /*If channel is Closed*/
    if(channel->isOpen == 1){
       pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
    /*If channel is Open*/
      /*Blocking other channels*/
      //pthread_cond_wait(&channel->RV,&channel->mutex);
    // 
     /*while( channel->buffer->size >= channel->buffer->capacity){
        pthread_cond_wait(&channel->RV,&channel->mutex);
          buffer_remove(channel->buffer,data);
          //pthread_cond_wait(&channel->RV,&channel->mutex);
          //pthread_cond_wait(&channel->RV,&channel->mutex);
      }*/
      size_t capacity = channel->buffer->size;
      while( capacity == channel->buffer->capacity){
        pthread_cond_wait(&channel->RV,&channel->mutex);
        pthread_mutex_unlock(&channel->mutex);
      }
        buffer_add(channel->buffer,data);
        capacity = channel->buffer->size;
        pthread_cond_signal(&channel->SD);
        pthread_mutex_unlock(&channel->mutex);
        return SUCCESS;
    
}

// Writes data to the given channel
// This is a non-blocking call i.e., the function simply returns if the channel is full
// Returns SUCCESS for successfully writing data to the channel,
// CHANNEL_FULL if the channel is full and the data was not added to the buffer,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_non_blocking_send(channel_t* channel, void* data)
{
   /* IMPLEMENT THIS */
	pthread_mutex_lock(&channel->mutex);
    //channel is closed
    if(channel->isOpen == 1){
       pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
    /*If channel is Open*/
    else{
      size_t capacity = buffer_current_size(channel->buffer);
      if(capacity <= buffer_capacity(channel->buffer)){
        buffer_add(channel->buffer,data);
        capacity++;
      }  
        //pthread_cond_signal(&channel->RV);
        pthread_mutex_unlock(&channel->mutex);
        return SUCCESS;
    }
}

// Reads data from the given channel and stores it in the function’s input parameter data (Note that it is a double pointer)
// This is a non-blocking call i.e., the function simply returns if the channel is empty
// Returns SUCCESS for successful retrieval of data,
// CHANNEL_EMPTY if the channel is empty and nothing was stored in data,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_non_blocking_receive(channel_t* channel, void** data)
{
//channel is closed
    pthread_mutex_lock(&channel->mutex);
    if(channel->isOpen == 1){
       pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
    /*If channel is Open*/
    else{
      /*Blocking other channels*/
      //pthread_cond_wait(&channel->SD,&channel->mutex);
      size_t capacity = buffer_current_size(channel->buffer);
      if( capacity >= channel->bufferCapacity){
          buffer_remove(channel->buffer,data);
          capacity++;
      }   
        //pthread_cond_signal(&channel->SD);
        pthread_mutex_unlock(&channel->mutex);
        return SUCCESS;
    }
}

// Closes the channel and informs all the blocking send/receive/select calls to return with CLOSED_ERROR
// Once the channel is closed, send/receive/select operations will cease to function and just return CLOSED_ERROR
// Returns SUCCESS if close is successful,
// CLOSED_ERROR if the channel is already closed, and
// GEN_ERROR in any other error case
enum channel_status channel_close(channel_t* channel)
{
  pthread_mutex_lock(&channel->mutex);
  /*If channel is close*/
  if(channel->isOpen == 1)
  {
    pthread_mutex_unlock(&channel->mutex);
    return CLOSED_ERROR;
  }
    channel->isOpen = 1;/*Closing the channel*/
    pthread_cond_broadcast(&(channel->SD));
    pthread_cond_broadcast(&(channel->RV));
    /* IMPLEMENT THIS */
    pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
}

// Frees all the memory allocated to the channel
// The caller is responsible for calling channel_close and waiting for all threads to finish their tasks before calling channel_destroy
// Returns SUCCESS if destroy is successful,
// DESTROY_ERROR if channel_destroy is called on an open channel, and
// GEN_ERROR in any other error case
enum channel_status channel_destroy(channel_t* channel)
{
  /*If channel is closed*/
  if(channel->isOpen == 0){
    return DESTROY_ERROR;
  }
  else{
    buffer_free(channel->buffer);/*Free the channel buffer*/
    pthread_mutex_destroy(&(channel->mutex));/*Destroy the mutex*/
    pthread_cond_destroy(&(channel->RV));/*Destroy the Condition Variable*/
    pthread_cond_destroy(&(channel->SD));/*Destroy the Condition Variable*/
    free(channel->data);/*Free Data*/
    free(channel);/*Free Channel*/
    return SUCCESS;
  }
    /* IMPLEMENT THIS */

}

// Takes an array of channels, channel_list, of type select_t and the array length, channel_count, as inputs
// This API iterates over the provided list and finds the set of possible channels which can be used to invoke the required operation (send or receive) specified in select_t
// If multiple options are available, it selects the first option and performs its corresponding action
// If no channel is available, the call is blocked and waits till it finds a channel which supports its required operation
// Once an operation has been successfully performed, select should set selected_index to the index of the channel that performed the operation and then return SUCCESS
// In the event that a channel is closed or encounters any error, the error should be propagated and returned through select
// Additionally, selected_index is set to the index of the channel that generated the error
enum channel_status channel_select(select_t* channel_list, size_t channel_count, size_t* selected_index)
{
    /* IMPLEMENT THIS */
    return SUCCESS;
}
