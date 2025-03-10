#include<iostream>
using namespace std;

int main(int argc, char* argv[])
{
  if ( 3 != argc )
    {
      cout << "Usage: " << argv[0] << " <satellites/holes> <multiplier> " << endl;
      exit(1);
    }

  size_t n = atoi(argv[1]);
  size_t m = atoi(argv[2]);

  
  size_t direction = m*n + 2;
  size_t satellite = n;
  size_t mode = n;
  size_t dummy = n;
  size_t instrument = n;
  size_t time = 2*m + 1;

  cout << "// ZAP encoding for satellite domain." << endl
       << "// The calibration target predicate is removed" << endl
       << "// Homemade state- based encoding" << endl << endl;

  cout << "SORT satellite " << satellite << ";" << endl
       << "SORT direction " << direction << ";" << endl
       << "SORT instrument " << instrument << "; " << endl
       << "SORT dummy " << dummy << "; " << endl
       << "SORT mode " << mode << ";" << endl
       << "SORT time " << time << ";" << endl << endl;
 
  cout << "  // fluent predicates " << endl
       << "PREDICATE onBoard( instrument satellite ) ; " << endl
       << "PREDICATE supports( instrument mode ) ; " << endl << endl
    
       << "// fluents with time arguments  " << endl
       << "PREDICATE pointing( satellite direction time ) ; " << endl
       << "PREDICATE powerAvail( satellite time ) ; " << endl
       << "PREDICATE powerOn( instrument time ) ;	 " << endl
       << "PREDICATE calibrated( instrument time ) ; " << endl
       << "PREDICATE haveImage( direction dummy mode time ) ; " << endl
       << "PREDICATE next( time time ) ; " << endl << endl;
  
  cout << "FORALL( i t2 t1 ) EXISTS(s)  ~next[ t1 t2 ] ~powerOn[i t2] powerOn[i t1] onBoard[i s]  { t1 != t2 } ;"  << endl
       << "FORALL( i t2 t1 ) EXISTS(s)  ~next[ t1 t2 ] ~powerOn[i t2] powerOn[i t1] powerAvail[ s t1 ]	{ t1 != t2 } ;"  << endl
       << "FORALL( i t1 t2 ) EXISTS(s)  ~next[ t1 t2 ] calibrated[ i t2 ] ~calibrated[ i t1 ] onBoard[ i s ]	{ t1 != t2 } ; "  << endl
       << "FORALL( s t1 t2 ) EXISTS(i)  ~next[ t1 t2 ] powerAvail[ s t2 ] ~powerAvail[ s t1 ] onBoard[ i s ]	{ t1 != t2 } ;"  << endl
       << "FORALL( i t1 t2 ) EXISTS(s)  ~next[ t1 t2 ] calibrated[ i t2 ] ~calibrated[ i t1 ] powerAvail[ s t1 ] 	{ t1 != t2 } ;"  << endl
    
       << "FORALL( s t2 t1 ) EXISTS(i)  ~next[ t1 t2 ] ~powerAvail[s t2] powerAvail[s t1] onBoard[ i s ] 	{ t1 != t2 } ;"  << endl
       << "FORALL( i t1 t2 ) EXISTS(s)  ~next[ t1 t2 ] powerOn[ i t2 ] ~powerOn[ i t1 ] onBoard[ i s ] 	{ t1 != t2 } ;"  << endl
       << "FORALL( s t2 t1 ) EXISTS(i)  ~next[ t1 t2 ] ~powerAvail[s t2] powerAvail[s t1] powerOn[ i t1 ] 	{ t1 != t2 } ;"  << endl
    
       << "FORALL( i t1 t2 ) EXISTS( s )  ~next[ t1 t2 ] ~calibrated[ i t2 ] calibrated[ i t1 ] onBoard[ i s ] 	{ t1 != t2 } ;"  << endl
       << "FORALL( i t1 t2 ) EXISTS( s d )  ~next[ t1 t2 ] ~calibrated[ i t2 ] calibrated[ i t1 ] pointing[ s d t1 ] 	{ t1 != t2 } ;"  << endl
       << "FORALL( i t1 t2 ) ~next[ t1 t2 ] ~calibrated[ i t2 ] calibrated[ i t1 ] powerOn[ i t1 ] 	{ t1 != t2 } ;"  << endl
    
       << "FORALL( d m b t1 t2 ) EXISTS( i )  ~next[ t1 t2 ] ~haveImage[ d b m t2 ] haveImage[ d b m t1 ] calibrated[ i t1 ] 	{ t1 != t2 } ;"  << endl
       << "FORALL( d m b t1 t2 ) EXISTS( i s )  ~next[ t1 t2 ] ~haveImage[ d b m t2 ] haveImage[ d b m t1 ] onBoard[ i s ] 	{ t1 != t2 } ;"  << endl
       << "FORALL( d m b t1 t2 ) EXISTS( i )  ~next[ t1 t2 ] ~haveImage[ d b m t2 ] haveImage[ d b m t1 ] supports[ i m ] 	{ t1 != t2 } ;"  << endl
       << "FORALL( d m b t1 t2 ) EXISTS( i )  ~next[ t1 t2 ] ~haveImage[ d b m t2 ] haveImage[ d b m t1 ] powerOn[ i t1 ] 	{ t1 != t2 } ;"  << endl
       << "FORALL( d m b t1 t2 ) EXISTS( s )  ~next[ t1 t2 ] ~haveImage[ d b m t2 ] haveImage[ d b m t1 ] pointing[ s d t1 ] 	{ t1 != t2 } ;"  << endl << endl;

    
  cout << "// change of state" << endl
       << "FORALL( i s t1 t2 ) EXISTS ( d ) ~next[ t1 t2 ] ~powerOn[ i t1 ] ~onBoard[ i s ] ~calibrated[ i t2 ] pointing[ s d t1 ] ;" << endl
       << "FORALL( i s b d m t1 t2 ) ~next[ t1 t2 ] ~calibrated[ i t1 ] ~onBoard[ i s ] ~supports[ i m ] ~powerOn[ i t1 ] haveImage[ d b m t1 ] ~haveImage[ d b m t2 ] pointing[ s d t1 ] 	{ t1 != t2 };" << endl
       << "FORALL( i s b d m t1 t2 ) ~next[ t1 t2 ] ~calibrated[ i t1 ] ~onBoard[ i s ] ~supports[ i m ] ~powerOn[ i t1 ] haveImage[ d b m t1 ] ~haveImage[ d b m t2 ] pointing[ s d t2 ] 	{ t1 != t2 };" << endl
       << "// consistency of state" << endl
       << "FORALL( s d1 d2 t ) ~pointing[ s d1 t ] ~pointing[ s d2 t ]	{  d1 != d2 } ;" << endl
       << "FORALL( i s1 s2 ) ~onBoard[ i s1 ] ~onBoard[ i s2 ]  { s1 != s2 } ;" << endl << endl;
  


 

  cout << "// initial conditions " << endl;
  cout << "// all satellites start off pointing at direction 1 " << endl;
  for (size_t s=1; s <= satellite; s++) {
    cout << "pointing[ " << s << " 1 1 ] ; " << endl;
  }

  cout << endl << "// all satellites have power available " << endl;
  for (size_t s=1; s <= satellite; s++) 
    cout << "powerAvail[ " << s << " 1 ] ; " << endl;

  cout << endl << "// start with power off" << endl;
  for (size_t i=1; i <= instrument; i++)
    cout << "~powerOn[ " << i << " 1 ] ; " << endl;

  cout << endl << "// nothing is calibrated" << endl;
  for (size_t i=1; i <= instrument; i++)
    cout << "~calibrated[ " << i << " 1 ] ; " << endl;

  cout << endl << "// instruments support the only available mode" << endl;
  for (size_t i=1; i <= instrument; i++)
    cout << "supports[ " << i << " 1 ] ; " << endl;

  
  cout << "// map each instrument to a satellite " << endl;
  for (size_t s=1; s <= satellite; s++) {
    cout << "onBoard[ " << s << " " << s << " ] ; " << endl;
  }
  
  for (size_t d=1; d <= direction; d++) {
    cout << "~haveImage[ " << d << " 1 1 1 ] ; " << endl;
  }




  
  cout << endl << "// final conditions " << endl;

  for (size_t d=2; d <= direction; d++) {
    cout << "haveImage[ " << d << " 1 1 " << time << " ] ; " << endl;
  }

  
  cout << endl << "// next predicates" << endl;
  for (int t = 1; t <= (time-1); t++) {
    cout << "next[" << t << " " << t+1 << "] ; " ;
    cout << endl;
  }

  return 0;
}
