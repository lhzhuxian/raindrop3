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
vector<vector<float> > current;
vector<vector<float> > result;
vector<vector<float> > update;
vector<vector<int> > lanscape;
vector<vector<vector<pair<int, int> > > > direction;

pthread_barrier_t barrier;
vector<bool> flag;

bool check() {
  for(auto i : flag) {
    if(i) return true;
  }
  return false;
}
struct p_arg {
  int start;
  int end;
  int id;
  int time_step;
  int total_time;
  vector<float> upper;
  vector<float> lower;
  p_arg(int _start, int _end, int _id, int _time_step):		\
    start(_start), end(_end), id(_id), total_time(0), time_step(_time_step){
    upper = vector<float> (dimension, 0);
    lower = vector<float> (dimension, 0);
  }
};
void * simulation(void * argument) {
  p_arg * arg = (p_arg *)argument;
  int count = 1;
  float trickle = 0;
  int time_step = arg->time_step;
  while(check()) {
    count = (arg->end - arg->start + 1) * dimension;
    for(int i = arg->start; i <= arg->end; ++i) {
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
	   if(d.first < arg->start) {
	     arg->upper[d.second] += trickle;
	   } else if(d.first > arg->end) {
	     arg->lower[d.second] += trickle;
	   } else {
	     update[d.first][d.second] += trickle;
	   }
	 }
	}
      }
    }
    if(time_step > 0) time_step -= 1;
    arg->total_time += 1;
    pthread_barrier_wait(&barrier);

    if(arg->start > 0) {
      for(int j = 0; j < dimension; ++j) {
         update[arg->start - 1][j] += arg->upper[j];
         arg->upper[j] = 0;
       } 
    }

    if(arg->end < dimension -1) {
       for(int j = 0; j < dimension; ++j) {
	 update[arg->end + 1][j] += arg->lower[j];
	 arg->lower[j] = 0;
       }
    }
    
    pthread_barrier_wait(&barrier);

    for(int i = arg->start; i <= arg->end ; ++i) {
      for(int j = 0; j < dimension; ++j) {
	
	current[i][j] += update[i][j];
	update[i][j] = 0;
	if(current[i][j] == 0.0) {
	  count--;
	}
      }
    }
    
    flag[arg->id] = (time_step || count)?true:false;
    pthread_barrier_wait(&barrier);
  }
}
int main(int argc, char ** argv) {
  if(argc != 6) {
    cout << "./rainfall_sep <thread_num> <time_step> <absorp_rate> <dimension> <file>" << endl;
    exit(-1);
  }
  thread_num = stoi(argv[1]);
  int time_step = stoi(argv[2]);
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

  
 
  int size = dimension / thread_num;
  if(size < 2) {
    if(dimension % 2) {
      cout << "dimension should be even number" << endl;
      exit(-1);
    }
    thread_num = dimension / 2;
    size = 2;
  }
  p_arg ** arg = new p_arg*[thread_num];
  
  flag = vector<bool> (thread_num, true);
  pthread_barrier_init(&barrier, NULL, thread_num);
  pthread_t * thread = new pthread_t[thread_num];
  int start = 0;
  
  struct timeval start_time, end_time;
  gettimeofday(&start_time, NULL);
  for(int i = 0; i < thread_num; ++i) {
    arg[i] = new p_arg(start, start + size - 1, i, time_step);
    pthread_create(thread + i, NULL, &simulation, (void*)(arg[i]));
    start +=  size;
  }
  for(int i = 0; i < thread_num; ++i) {
    pthread_join(thread[i], NULL);
  }
  gettimeofday(&end_time, NULL);
  double run_time = calc_time(start_time, end_time);
  
  
  cout << "Rainfall simulation took " << arg[0]->total_time << \
    " time steps to complete." << endl;
  cout << "Runtime = " << run_time << " seconds\n" << endl;
  cout << "The following grid shows the number of raindrops absorbed at each point:" << endl;
  for (unsigned i=0; i < dimension; i++) {
    for (unsigned j=0; j < dimension; j++) {
      cout << setw(8) << setprecision(6) << result[i][j];
    }                                                  
    cout << endl;
  }     

  delete[] thread;
  for(int i = 0; i < thread_num; ++i) {
    delete arg[i];
  }
  delete[] arg;
}
