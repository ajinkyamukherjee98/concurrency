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
        channel->buffer = buffer_create(0);
        channel->isOpen = 0;/* to check if channel is free?*/
        //channel->bufferCapacity = 0;/*To know the original capacity of the buffer*/
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
        //channel->bufferCapacity = 0;/*To know the original capacity of the buffer*/
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
  pthread_mutex_lock(&(channel->mutex));
    //channel is closed
    if(channel->isOpen == 1){
       pthread_mutex_unlock(&(channel->mutex));
        return CLOSED_ERROR;
    }
    /*If channel is Open*/
       else {
          while(buffer_add(channel->buffer,data) != BUFFER_SUCCESS){/*Keep waiting if buffer is full*/
            pthread_cond_wait(&(channel->RV),&(channel->mutex));
            if(channel->isOpen == 1){/*Check if channel is closed after waiting:TA*/
                pthread_mutex_unlock(&(channel->mutex));
                  return CLOSED_ERROR;
              }
          }   
            pthread_cond_signal(&(channel->SD));/*Signal other threads*/
            pthread_mutex_unlock(&(channel->mutex));
            return SUCCESS;
       }
          pthread_mutex_unlock(&(channel->mutex));/*If no cindition is satisfied: Return Genneric Error */
            return GEN_ERROR;
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
    pthread_mutex_lock(&(channel->mutex)); 
     /*If channel is Closed*/
    if(channel->isOpen == 1){
       pthread_mutex_unlock(&(channel->mutex));
        return CLOSED_ERROR;
    }
    /*If channel is Open*/
      
      else {        
        while(buffer_remove(channel->buffer,data) != BUFFER_SUCCESS){/*Keep waiting if buffer is empty*/
          pthread_cond_wait(&(channel->SD),&(channel->mutex));
          /*As per TA: Check after waitig if channel is open or not*/
          if(channel->isOpen == 1){
            pthread_mutex_unlock(&(channel->mutex));
            return CLOSED_ERROR;
          }
        }
        pthread_cond_signal(&(channel->RV));/*Signal other threads*/
        pthread_mutex_unlock(&(channel->mutex));
        return SUCCESS;
              
      }
      
      pthread_mutex_unlock(&(channel->mutex));/*If no cindition is satisfied: Return Genneric Error */
      return GEN_ERROR;
    
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
  pthread_mutex_lock(&(channel->mutex));
    //channel is closed
    if(channel->isOpen == 1){
       pthread_mutex_unlock(&(channel->mutex));
        return CLOSED_ERROR;
    }
    /*If channel is Open*/
       else {
          if(buffer_add(channel->buffer,data) != BUFFER_SUCCESS){/*unlock & Return CHANNEL_FULL if Channel is full*/
           pthread_mutex_unlock(&(channel->mutex));
            return CHANNEL_FULL;
          }   
            //pthread_cond_signal(&(channel->SD));/*Signal other threads*/
            pthread_mutex_unlock(&(channel->mutex));
            return SUCCESS;
       }
          pthread_mutex_unlock(&(channel->mutex));/*If no cindition is satisfied: Return Genneric Error */
            return GEN_ERROR;
}

// Reads data from the given channel and stores it in the function’s input parameter data (Note that it is a double pointer)
// This is a non-blocking call i.e., the function simply returns if the channel is empty
// Returns SUCCESS for successful retrieval of data,
// CHANNEL_EMPTY if the channel is empty and nothing was stored in data,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_non_blocking_receive(channel_t* channel, void** data)
{
  /* IMPLEMENT THIS */
    //channel is closed
    pthread_mutex_lock(&(channel->mutex)); 
     /*If channel is Closed*/
    if(channel->isOpen == 1){
       pthread_mutex_unlock(&(channel->mutex));
        return CLOSED_ERROR;
    }
    /*If channel is Open*/
      
      else {        
      if(buffer_remove(channel->buffer,data) != BUFFER_SUCCESS){/*unlock & Return CHANNEL_EMPTY if Channel is full*/
        pthread_mutex_unlock(&(channel->mutex));
        return CHANNEL_EMPTY;
      }
        pthread_cond_signal(&(channel->RV));/*Signal other threads*/
        pthread_mutex_unlock(&(channel->mutex));
        return SUCCESS;      
      }
      pthread_mutex_unlock(&(channel->mutex));/*If no cindition is satisfied: Return Genneric Error */
      return GEN_ERROR;
      
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
  }else{
    channel->isOpen = 1;/*Closing the channel*/
    pthread_cond_broadcast(&(channel->SD));
    pthread_cond_broadcast(&(channel->RV));
    /* IMPLEMENT THIS */
    pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
  }
  pthread_mutex_unlock(&channel->mutex);
  return GEN_ERROR;
}

// Frees all the memory allocated to the channel
// The caller is responsible for calling channel_close and waiting for all threads to finish their tasks before calling channel_destroy
// Returns SUCCESS if destroy is successful,
// DESTROY_ERROR if channel_destroy is called on an open channel, and
// GEN_ERROR in any other error case
enum channel_status channel_destroy(channel_t* channel)
{
  /*If channel is Open*/
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
    return GEN_ERROR;

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
   /*
  Chanel_list has both channels and direction
  Assume you have a chanel in list, for each cahnnel there will be send or recv, it shuld not ideally wait.
  If the buffer is full for that channel & direction is send-> Cannot perform on this channel.
 Go to the next channel.
 if the buffer is empty & direction is RCV, it cannot perform on that channel, go to the next channel.
 once all channels are checked and nothing can be done we wait(). Wake up the select function by send or recieve function.
 Once it wakes up so it starts all over again, if it finds something it can perform return SUCCESS. 
 If any of the channels is closed return CLOSED_ERROR; 
  */
  /*Declared locally so that all channels can follow themutex and cond variables*/
  // Declare mutex and then initialize it. Do the same for condtionla variable.
 //Need to make all channels know where is the conditional variable. 
  pthread_mutex_t m;// Declaring a mutex
  pthread_cond_t R; // Declaring Recieve
  pthread_cond_t S; // Declaring Send
  size_t chosen_Index = 1;
  while(chosen_Index <= channel_count){
    pthread_mutex_lock(&m);// Lock before sharing
    //*selected_index = chosen_Index;
 /*if(channel_list[chosen_Index].channel->isOpen == 1){
   pthread_mutex_unlock(&m);
   return CLOSED_ERROR;
 }*/
 /* If the buffer is full for that channel & direction is send-> Cannot perform on this channel.*/
if(channel_list->dir == SEND){//channel_list[chosen_Index].channel->buffer,channel_list[chosen_Index].data) != BUFFER_SUCCESS){// && channel_list->dir == SEND){
   /*Go to the Next Channel*/

   if(channel_non_blocking_send(channel_list[chosen_Index].channel,channel_list[chosen_Index].data) == CHANNEL_FULL){
      chosen_Index = chosen_Index +1;
     *selected_index = chosen_Index; 
     
     //chosen_Index = chosen_Index +1; 
     pthread_mutex_unlock(&m);
   }
   else{
     *selected_index = chosen_Index;
     pthread_mutex_unlock(&m);
     return SUCCESS;
   }
     pthread_mutex_destroy(&m);/*Destroy the mutex*/
    pthread_cond_destroy(&R);/*Destroy the Condition Variable*/
    pthread_cond_destroy(&S);/*Destroy the Condition Variable*/
 }
 /* If the buffer is empty & direction is RECV, it cannot perform on that channel, go to the next channel.*/
 if(channel_list->dir == RECV){// && channel_list->dir == RECV){
   if(channel_non_blocking_receive(channel_list[chosen_Index].channel,channel_list[chosen_Index].data) == CHANNEL_EMPTY){
      chosen_Index = chosen_Index +1; 
     *selected_index = chosen_Index +1; 
      
        pthread_mutex_unlock(&m);
   }
   else{
     *selected_index = chosen_Index;
     pthread_mutex_unlock(&m);
     return SUCCESS;
   }
    pthread_mutex_destroy(&m);/*Destroy the mutex*/
    pthread_cond_destroy(&R);/*Destroy the Condition Variable*/
    pthread_cond_destroy(&S);/*Destroy the Condition Variable*/
  }
    
   // Once an operation has been successfully performed, select should set selected_index to the index of the channel that performed the operation and then return SUCCESS
  }
  return GEN_ERROR;
}

