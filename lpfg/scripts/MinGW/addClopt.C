#include <fstream>
#include <iostream>
#include <string>
using namespace std;

const char* clopt = "clopt";

int main(int ac,char **av)
{
  ifstream infile(clopt);
  string cloptString;
  getline(infile,cloptString);
  infile.close();

  for(int i = 0 ; i < cloptString.length() ;
      i = cloptString.find('/',i))
    cloptString.replace(i,1,1,'-');

  cerr << cloptString << endl;

  if(ac > 1)
  {
    string cmdline(av[1]);
    cmdline += " ";
    cmdline += cloptString;

    for(int i = 2; i < ac ; i++)
    {
      cmdline += " ";
      cmdline += av[i];
    }

    cerr << cmdline << endl;
    system(cmdline.data());
  }
  return 0;
}
