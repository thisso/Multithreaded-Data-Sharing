#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include <pthread.h>
#include <unistd.h>  // for sleep
#include "./grep_request.hpp"
#include "./RequestQueue.hpp"

using namespace std;

// A helper structure to pass the shared RequestQueue to threads.
struct ThreadArgs {
  RequestQueue* queue;
};

// Utility function: Split a string on whitespace into non-empty tokens.
vector<string> split(const string& input) {
  vector<string> tokens;
  size_t start = 0;
  size_t pos = input.find_first_of(" \r\t\n");
  while (pos != string::npos) {
    if (pos > start) {
      tokens.push_back(input.substr(start, pos - start));
    }
    start = pos + 1;
    pos = input.find_first_of(" \r\t\n", start);
  }
  if (start < input.size()) {
    tokens.push_back(input.substr(start));
  }
  return tokens;
}

// Helper function: Returns true if the line (when split into tokens)
// contains every query exactly.
bool lineMatches(const string& line, const vector<string>& queries) {
  vector<string> lineTokens = split(line);
  // For each query, ensure it appears as an exact token in the line.
  for (const auto& query : queries) {
    bool found = false;
    for (const auto& token : lineTokens) {
      if (token == query) {
        found = true;
        break;
      }
    }
    if (!found)
      return false;
  }
  return true;
}

// Reader thread: Reads lines from stdin, tokenizes each into a grep_request,
// and enqueues it into the shared RequestQueue. Sleeps for 1 second after each.
void* readerThread(void* arg) {
  ThreadArgs* args = static_cast<ThreadArgs*>(arg);
  RequestQueue* queue = args->queue;
  string line;
  while (getline(cin, line)) {
    vector<string> tokens = split(line);
    if (tokens.size() < 2) {
      cerr << "Parsing Error: expected a file name followed by at least one query" << endl;
      continue;
    }
    grep_request req;
    req.fname = tokens.front();
    tokens.erase(tokens.begin());
    req.queries = tokens;
    
    if (!queue->add(req)) {
      // If queue is closed, break.
      break;
    }
    sleep(1);
  }
  // Signal that no more requests will be added.
  queue->close();
  return nullptr;
}

// Processor thread: Continuously removes a grep_request (blocking if needed)
// and processes it by printing out the header then printing every file line
// that contains all query tokens (using exact token matching).
void* processorThread(void* arg) {
  ThreadArgs* args = static_cast<ThreadArgs*>(arg);
  RequestQueue* queue = args->queue;
  while (true) {
    optional<grep_request> reqOpt = queue->wait_remove();
    if (!reqOpt.has_value()) {
      // The queue is closed and empty.
      break;
    }
    grep_request req = reqOpt.value();
    
    // Print the header (filename and query tokens).
    cout << req.to_string() << endl;
    
    ifstream file(req.fname);
    if (!file.is_open()) {
      // If unable to open the file, continue to next request.
      continue;
    }
    string line;
    while (getline(file, line)) {
      if (lineMatches(line, req.queries))
        cout << line << endl;
    }
  }
  return nullptr;
}

int main() {
  RequestQueue queue;
  ThreadArgs args;
  args.queue = &queue;
  
  pthread_t tidReader, tidProcessor;
  
  // Create the reader and processor threads.
  pthread_create(&tidReader, nullptr, readerThread, &args);
  pthread_create(&tidProcessor, nullptr, processorThread, &args);
  
  // Wait for both threads to finish.
  pthread_join(tidReader, nullptr);
  pthread_join(tidProcessor, nullptr);
  
  return 0;
}
