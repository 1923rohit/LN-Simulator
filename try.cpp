/* fopen example */
#include <bits/stdc++.h>
using namespace std;
int main ()
{
  ofstream myfile;
  myfile.open ("text_file.txt");
  for(int i=0;i<5;i++)myfile<<i<<" gg\n";
  myfile.close();
  return 0;
}