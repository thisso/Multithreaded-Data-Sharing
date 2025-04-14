#include "./RequestQueue.hpp"
#include <pthread.h>
#include <optional>
#include <cstdlib>

using std::optional;
using std::nullopt;

RequestQueue::RequestQueue() : head(nullptr), tail(nullptr), count(0), closed(false) {
  pthread_mutex_init(&mutex, nullptr);
  pthread_cond_init(&cond, nullptr);
}

RequestQueue::~RequestQueue() {
  // Free any remaining nodes in the queue.
  while (head) {
    QueueNode* temp = head;
    head = head->next;
    delete temp;
  }
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);
}

bool RequestQueue::add(grep_request val) {
  pthread_mutex_lock(&mutex);
  if (closed) {
    pthread_mutex_unlock(&mutex);
    return false;
  }
  QueueNode* node = new QueueNode(val);
  if (!head) {
    head = node;
    tail = node;
  } else {
    tail->next = node;
    tail = node;
  }
  count++;
  // Signal any threads waiting in wait_remove() that a new request is available.
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
  return true;
}

void RequestQueue::close() {
  pthread_mutex_lock(&mutex);
  closed = true;
  // Wake up all threads blocked in wait_remove().
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mutex);
}

optional<grep_request> RequestQueue::remove() {
  pthread_mutex_lock(&mutex);
  if (head == nullptr) {
    pthread_mutex_unlock(&mutex);
    return nullopt;
  }
  QueueNode* node = head;
  head = head->next;
  if (head == nullptr) {
    tail = nullptr;
  }
  count--;
  grep_request ret = node->value;
  delete node;
  pthread_mutex_unlock(&mutex);
  return ret;
}

optional<grep_request> RequestQueue::wait_remove() {
  pthread_mutex_lock(&mutex);
  while (head == nullptr && !closed) {
    // Wait until a request is available or the queue is closed.
    pthread_cond_wait(&cond, &mutex);
  }
  if (head == nullptr) {
    // If queue is closed and empty, return nullopt.
    pthread_mutex_unlock(&mutex);
    return nullopt;
  }
  QueueNode* node = head;
  head = head->next;
  if (head == nullptr)
    tail = nullptr;
  count--;
  grep_request ret = node->value;
  delete node;
  pthread_mutex_unlock(&mutex);
  return ret;
}

int RequestQueue::length() {
  pthread_mutex_lock(&mutex);
  int len = count;
  pthread_mutex_unlock(&mutex);
  return len;
}
