#include <NTL/ZZ_pXFactoring.h>

using namespace std;
using namespace NTL;

int main()
{
   ZZ p;
   cin >> p;
   ZZ_p::init(p);

   ZZ_pX f;
   cin >> f;

   Vec< Pair< ZZ_pX, long > > factors;

   CanZass(factors, f);  // calls "Cantor/Zassenhaus" algorithm

   cout << factors << "\n";
   return 0;
}

