#include <fstream>
#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <limits.h>
#include <algorithm>
#include <time.h>
#include <sys/time.h>
#include <iomanip>

using namespace std;

double calc_time(struct timeval start, struct timeval end) {
  double start_sec = (double)start.tv_sec + (double)start.tv_usec / 1000000.0 ;
  double end_sec = (double)end.tv_sec + (double)end.tv_usec / 1000000.0;

  if (end_sec < start_sec) {
    return 0;
  } else {
    return end_sec - start_sec;
  }
}

void PrintMatrix(vector<vector<float> > & current) {
  for(auto & i : current) {
    for(auto j :i) {
      cout << j << ' ';
    }
    cout << endl;
  }
}

int thread_num;
float absorp_rate;
int dimension;
int time_step;
vector<vector<float> > current;
vector<vector<float> > result;
vector<vector<float> > update;
vector<vector<int> > lanscape;
vector<vector<vector<pair<int, int> > > > direction;


int simulation() {
  int count = 1;
  float trickle = 0;
  int total_time;
  while(time_step || count) {
    count = dimension * dimension;
    for(int i = 0; i < dimension; ++i) {
      for(int j = 0; j < dimension; ++j) {
	if(time_step > 0) current[i][j]++;
	if(current[i][j] > absorp_rate) {
          current[i][j] -= absorp_rate;
          result[i][j] += absorp_rate;
	} else  {
	  result[i][j] += current[i][j];
	  current[i][j] = 0;
	  
	}
	if(direction[i][j].size() > 0) {
	  if(current[i][j] > 1.0) {
	    current[i][j] -= 1.0;
	    trickle = 1.0 / direction[i][j].size();
	  } else {
	    trickle = current[i][j] / direction[i][j].size();
	    current[i][j] = 0.0;
	  }
	 for(auto d: direction[i][j]) {
	   update[d.first][d.second] += trickle;
	 }
	}
      }
    }
    if(time_step > 0) time_step -= 1;
    total_time += 1;
    

    for(int i = 0; i < dimension ; ++i) {
      for(int j = 0; j < dimension; ++j) {
	
	current[i][j] += update[i][j];
	update[i][j] = 0;
	if(current[i][j] == 0.0) {
	  count--;
	}
      }
    }
  }
  return total_time;
}
int main(int argc, char ** argv) {
  if(argc != 6) {
    cout << "./rainfall_sep <thread_num> <time_step> <absorp_rate> <dimension> <file>" << endl;
    exit(-1);
  }
  thread_num = stoi(argv[1]);
  time_step = stoi(argv[2]);
  absorp_rate = stof(argv[3]);
  dimension = stoi(argv[4]);
  
  current =  vector<vector<float> >(dimension,				\
				    vector<float>(dimension, 0));
  result =  vector<vector<float> > (dimension,				\
				    vector<float>(dimension, 0));
  update =  vector<vector<float> >(dimension,			\
				   vector<float>(dimension, 0));
  lanscape = vector<vector<int> > (dimension,			\
				   vector<int>(dimension, 0));
  direction = vector<vector<vector<pair<int, int> > > >(dimension, vector<vector<pair<int, int> > >(dimension));
  
  string input_file = argv[5];
  ifstream fin(input_file);
  for(int i = 0; i < dimension; ++i) {
    for(int j = 0; j < dimension; ++j) {
      fin >> lanscape[i][j];
    }
  }


  vector<pair<int, int> > neighbur;
  neighbur.push_back({-1, 0});
  neighbur.push_back({1, 0});
  neighbur.push_back({0, -1});
  neighbur.push_back({0, 1});
  for(int i = 0; i < dimension; ++i) {
    for(int j = 0; j < dimension; ++j) {
      
      int min_neighbur = lanscape[i][j];
      for(int g = 0; g <= neighbur.size(); g++) {
	
	  int x = i + neighbur[g].first;
	  int y = j + neighbur[g].second;
	  if(x >= 0 && x < dimension &&	y >= 0 && y < dimension )  {
	    min_neighbur = min(min_neighbur, lanscape[x][y]);
	  }
	  
      }
      if(min_neighbur < lanscape[i][j]) {
	for(int g = 0; g <= neighbur.size(); g++) {
	  int x = i + neighbur[g].first;
	  int y = j + neighbur[g].second;
	  if(x >= 0 && x < dimension && y >= 0 && y < dimension &&	  
	     lanscape[x][y] == min_neighbur) {
	    direction[i][j].push_back({x, y});
	  }
	}
      }
      
    }
  }

  
   
  struct timeval start_time, end_time;
  gettimeofday(&start_time, NULL);
  int total_time = simulation();
  gettimeofday(&end_time, NULL);
  double run_time = calc_time(start_time, end_time);
  
  
  cout << "Rainfall simulation took " << total_time << \
    " time steps to complete." << endl;
  cout << "Runtime = " << run_time << " seconds\n" << endl;
  cout << "The following grid shows the number of raindrops absorbed at each point:" << endl;
  for (unsigned i=0; i < dimension; i++) {
    for (unsigned j=0; j < dimension; j++) {
      cout << setw(8) << setprecision(6) << result[i][j];
    }                                                  
    cout << endl;
  }
  
}
