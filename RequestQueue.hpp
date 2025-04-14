#ifndef REQUEST_QUEUE_HPP_
#define REQUEST_QUEUE_HPP_

#include <pthread.h>
#include <optional>
#include "./grep_request.hpp"

///////////////////////////////////////////////////////////////////////////////
// A RequestQueue represents a thread-safe queue of grep_request values.
//
// It supports adding requests, removing requests (non-blocking),
// and a blocking wait_remove() that waits until a request is available.
// When the queue is closed, add() will fail and waiting removals
// will return nullopt once the queued items are exhausted.
///////////////////////////////////////////////////////////////////////////////
class RequestQueue {
 public:
  // Constructor: Initialize an empty queue ready for concurrent operations.
  RequestQueue();

  // Destructor: Clean up any remaining nodes and synchronization primitives.
  ~RequestQueue();

  // Adds a grep_request to the end of the queue.
  // Returns true if the addition is successful, or false if the queue is closed.
  bool add(grep_request val);

  // Closes the queue. After closing, add() calls fail.
  // Waiting threads on wait_remove() will be woken and return nullopt if the queue is empty.
  void close();

  // Removes a grep_request from the front of the queue (non-blocking).
  // Returns the removed request if available, or nullopt if the queue is empty.
  std::optional<grep_request> remove();

  // Removes a grep_request from the front of the queue.
  // If the queue is empty, the calling thread will block until a request is available,
  // unless the queue is closed (in which case, returns nullopt).
  std::optional<grep_request> wait_remove();

  // Returns the number of requests currently in the queue.
  int length();

  // Delete copy/move constructors and assignment operators.
  RequestQueue(const RequestQueue& other) = delete;
  RequestQueue& operator=(const RequestQueue& other) = delete;
  RequestQueue(RequestQueue&& other) = delete;
  RequestQueue& operator=(RequestQueue&& other) = delete;

 private:
  // Internal linked list node for the queue.
  struct QueueNode {
    grep_request value;
    QueueNode* next;
    QueueNode(const grep_request& req) : value(req), next(nullptr) {}
  };

  QueueNode* head;  // Pointer to the first node.
  QueueNode* tail;  // Pointer to the last node.
  int count;        // Number of elements in the queue.
  bool closed;      // Set to true when the queue is closed.

  pthread_mutex_t mutex;  // Mutex for synchronizing access.
  pthread_cond_t cond;    // Condition variable for waiting in wait_remove().
};

#endif  // REQUEST_QUEUE_HPP_
