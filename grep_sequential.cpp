#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include "./grep_request.hpp"

using namespace std;

// Splits the input string on whitespace and returns all non-empty tokens.
vector<string> split(const string& input) {
  vector<string> tokens;
  size_t start = 0;
  size_t pos = input.find_first_of(" \r\t\n");
  while (pos != string::npos) {
    if (pos > start)
      tokens.push_back(input.substr(start, pos - start));
    start = pos + 1;
    pos = input.find_first_of(" \r\t\n", start);
  }
  if (start < input.size())
    tokens.push_back(input.substr(start));
  return tokens;
}

// Returns true if, when the line is split into tokens, every query token is present exactly.
bool lineMatches(const string& line, const vector<string>& queries) {
  vector<string> tokens = split(line);
  for (const auto& query : queries) {
    bool found = false;
    for (const auto& token : tokens) {
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

// Given a grep_request, prints the request header and then
// prints each line in the specified file that contains all the query tokens exactly.
void grep(const grep_request& request) {
  // Print the request header.
  cout << request.to_string() << endl;

  ifstream file(request.fname);
  if (!file.is_open()) {
    // If the file cannot be opened, simply return.
    return;
  }

  string line;
  while (getline(file, line)) {
    // Only print the line if every search term exists as an exact token.
    if (lineMatches(line, request.queries))
      cout << line << endl;
  }
}

// Reads a query from the console and returns it.
// Retries if the input is not valid (at least a filename and one search term).
// Returns nullopt when EOF (ctrl + d) is encountered.
optional<grep_request> read_query() {
  grep_request request;
  string line;
  getline(cin, line);
  
  while (cin) {
    vector<string> tokens = split(line);
    if (tokens.size() > 1) {
      request.fname = tokens.at(0);
      tokens.erase(tokens.begin());
      request.queries = std::move(tokens);
      return request;
    }
    cerr << "Parsing Error: expected a file name followed by one or more tokens to search for" << endl;
    getline(cin, line);
  }
  
  return nullopt;
}

int main() {
  auto opt = read_query();
  while (opt.has_value()) {
    grep(opt.value());
    opt = read_query();
  }
  return EXIT_SUCCESS;
}
