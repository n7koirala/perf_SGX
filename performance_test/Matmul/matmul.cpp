#include <iostream>
#include <cstdlib>
#include <unistd.h>

using namespace std;


// This function multiplies
// mat1[][] and mat2[][], and
// stores the result in res[][]
void multiply(int **mat1,
              int **mat2,
              int **res, int N)
{
    int i, j, k;
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            res[i][j] = 0;
            for (k = 0; k < N; k++){res[i][j] += mat1[i][k] * mat2[k][j];}
            //cout << res[i][j] << " ";
        }

    }
}

// Driver Code
int main(int argc, char * argv[])
  {
    int N = 512;
    srand(0);
    int **res = new int*[N]; // To store result
    int **mat1 = new int*[N]; // To store result
    int **mat2 = new int*[N]; // To store result
    for(int i =0; i < N; ++i)
    {
      res[i]= new int[N];
      mat1[i]= new int[N];
      mat2[i]= new int[N];
      for(int j = 0; j < N; j++)
      {
	       res[i][j] = 0;
	       mat1[i][j] = (rand()%1000);
	       mat2[i][j] = (rand()%1000);
      }
    }

    multiply(mat1, mat2, res, N);

    //cout << "Computation is complete!" << endl;
    return 0;
}
