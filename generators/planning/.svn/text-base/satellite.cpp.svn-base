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
  size_t mode = 1;
  size_t instrument = n;
  size_t time = 2*m + 2;

  cout << "// ZAP encoding for satellite domain." << endl
       << "// The calibration target predicate is removed" << endl
       << "// Based on graph plan based encoding" << endl << endl;
  
  cout << "SORT satellite " << satellite << ";" << endl
       << "SORT direction " << direction << ";" << endl
       << "SORT instrument " << instrument << "; " << endl
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
       << "PREDICATE haveImage( direction mode time ) ; " << endl
       << "PREDICATE next( time time ) ; " << endl << endl
    
       << "// action predicates " << endl
       << "PREDICATE turnTo( satellite direction direction time ) ; " << endl
       << "PREDICATE switchOn( instrument satellite time ) ; " << endl
       << "PREDICATE switchOff( instrument satellite time ) ; " << endl
       << "PREDICATE calibrate( instrument satellite direction time ) ; " << endl
       << "PREDICATE takeImage( instrument satellite direction mode time ) ; " << endl << endl;
  

  cout << "// turnTo preconditions depth: 1" << endl
       << "FORALL( s d1 d2 t ) ~turnTo[ s d1 d2 t ] pointing[s d1 t] ;" << endl << endl
    
       << "// switchOn preconditions depth: 1" << endl 
       << "FORALL( i s t ) ~switchOn[ i s t ] onBoard[ i s ] ;" << endl 
       << "FORALL( i s t ) ~switchOn[ i s t ] powerAvail[ s t ] ;" << endl << endl
    
       << "// switchOff preconditions depth: 1,0" << endl 
       << "FORALL( i s t ) ~switchOff[ i s t ] onBoard[ i s ] ;" << endl 
       << "FORALL( i s t ) ~switchOff[ i s t ] powerOn[ i t ] ;" << endl << endl
    
       << "// calibrate preconditions depth: 2,2,1,2" << endl 
       << "FORALL( i s d t ) ~calibrate[ i s d t ] onBoard[ i s ] ;" << endl 
       << "FORALL( i s d t ) ~calibrate[ i s d t ] pointing[ s d t ] ;" << endl 
       << "FORALL( i s d t ) ~calibrate[ i s d t ] powerOn[ i t ] ;" << endl << endl
  
       << "// takeImage preconditions depth: 3,3,3,3,2" << endl 
       << "FORALL( i s d m t ) ~takeImage[ i s d m t ] calibrated[ i t ] ;" << endl 
       << "FORALL( i s d m t ) ~takeImage[ i s d m t ] onBoard[ i s ] ;" << endl 
       << "FORALL( i s d m t ) ~takeImage[ i s d m t ] supports[ i m ] ;" << endl 
       << "FORALL( i s d m t ) ~takeImage[ i s d m t ] powerOn[ i t ] ;" << endl 
       << "FORALL( i s d m t ) ~takeImage[ i s d m t ] pointing[ s d t ] ;" << endl << endl;

  


  cout << "// facts imply disjunction of add effect 	all depth: 1" << endl 
       << "FORALL( s d2 t2 t1 ) EXISTS(d1) ~next[ t1 t2 ] ~pointing[ s d2 t2 ] pointing[ s d2 t1 ] turnTo[ s d1 d2 t1 ]" << endl  
       << "	{" << endl 
       << "		t1 != t2" << endl 
       << "	} ;" << endl 
    
       << "FORALL( s d1 t2 t1 ) EXISTS(d2)  ~next[ t1 t2 ] pointing[ s d1 t2 ] ~pointing[ s d1 t1 ] turnTo[ s d1 d2 t1 ]" << endl  
       << "	{" << endl 
       << "		t1 != t2" << endl 
       << "	} ;" << endl
    
       << "FORALL( i t2 t1 ) EXISTS(s)  ~next[ t1 t2 ] ~powerOn[i t2] powerOn[i t1] switchOn[ i s t1 ]" << endl  
       << "	{" << endl 
       << "		t1 != t2" << endl 
       << "	} ;" << endl
    
       << "FORALL( i t1 t2 ) EXISTS(s)  ~next[ t1 t2 ] calibrated[ i t2 ] ~calibrated[ i t1 ] switchOn[ i s t1 ] " << endl 
       << "	{" << endl 
       << "		t1 != t2" << endl 
       << "	} ;" << endl
    
       << "FORALL( s t1 t2 ) EXISTS(i)  ~next[ t1 t2 ] powerAvail[ s t2 ] ~powerAvail[ s t1 ] switchOn[ i s t1 ]" << endl  
       << "	{" << endl 
       << "		t1 != t2" << endl 
       << "	} ;" << endl 
    
       << "FORALL( s t2 t1 ) EXISTS(i)  ~next[ t1 t2 ] ~powerAvail[s t2] powerAvail[s t1] switchOff[ i s t1 ] " << endl 
       << "	{" << endl 
       << "		t1 != t2" << endl 
       << "	} ;" << endl 
    
       << "FORALL( i t1 t2 ) EXISTS(s)  ~next[ t1 t2 ] powerOn[ i t2 ] ~powerOn[ i t1 ] switchOff[ i s t1 ] " << endl 
       << "	{" << endl 
       << "		t1 != t2" << endl 
       << "	} ;" << endl 
    
       << "FORALL( i t1 t2 ) EXISTS( s d )  ~next[ t1 t2 ] ~calibrated[ i t2 ] calibrated[ i t1 ] calibrate[ i s d t1 ] " << endl 
       << "	{" << endl 
       << "		t1 != t2" << endl 
       << "	} ;" << endl 
    
       << "FORALL( d m t1 t2 ) EXISTS( i s )  ~next[ t1 t2 ] ~haveImage[ d m t2 ] haveImage[ d m t1 ] takeImage[ i s d m t1 ] " << endl 
       << "	{" << endl 
       << "		t1 != t2" << endl 
       << "	} ;" << endl << endl;
  


  cout << "// conflicting actions are mutually exclusive" << endl
       << "FORALL( s d1 d2 d3 t ) ~turnTo[ s d1 d2 t ] ~turnTo[ s d1 d3 t ]" << endl 
       << "	{" << endl
       << "		d2 != d3" << endl
       << "	} ;" << endl
       << "FORALL( s d1 d2 d3 t ) ~turnTo[ s d1 d2 t ] ~turnTo[ s d3 d2 t ] " << endl
       << "	{" << endl
       << "		d1 != d3" << endl
       << "	} ;" << endl
       << "FORALL( s d1 d2 d3 d4 t ) ~turnTo[ s d1 d2 t ] ~turnTo[ s d3 d4 t ] " << endl
       << "	{" << endl
       << "		d1 != d3," << endl
       << "		d2 != d4" << endl
       << "	} ;" << endl
       << "FORALL( s d1 d2 d3 i t ) ~turnTo[ s d1 d2 t ] ~calibrate[ i s d3 t ] ;" << endl
       << "FORALL( s d1 d2 d3 i m t ) ~turnTo[ s d1 d2 t ] ~takeImage[ i s d3 m t ] ; " << endl
       << "FORALL( i s d t ) ~switchOn[ i s t ] ~calibrate[ i s d t ] ;" << endl
       << "FORALL( i s t ) ~switchOn[ i s t ] ~switchOff[ i s t ] ;" << endl
       << "FORALL( i s d m t ) ~switchOn[ i s t ] ~takeImage[ i s d m t ] ;" << endl
       << "FORALL( i s d t ) ~switchOff[ i s t ] ~calibrate[ i s d t ] ;" << endl
       << "FORALL( i s d m t ) ~switchOff[ i s t ] ~takeImage[ i s d m t ] ;" << endl
       << "FORALL( i s d1 d2 m t ) ~takeImage[ i s d1 m t ] ~calibrate[ i s d2 t ] ;" << endl << endl;
  



 

  cout << "// initial conditions " << endl;
  cout << "// all satellites start off pointing at direction 1 " << endl;
  for (size_t s=1; s <= satellite; s++) {
    cout << "pointing[ " << s << " 1 1 ] ; " << endl;
    for (size_t d=2; d <= direction; d++)
      cout << "~pointing[ " << s << " " << d << " 1 ] ; " << endl;
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
    for (size_t s2=1; s2 <= satellite; s2++)
      if (s != s2)
	cout << "~onBoard[ " << s << " " << s2 << " ] ; " << endl;
  }
  
  for (size_t d=1; d <= direction; d++) {
    cout << "~haveImage[ " << d << " 1 1 ] ; " << endl;
  }




  
  cout << endl << "// final conditions " << endl;

  for (size_t d=2; d <= direction; d++) {
    cout << "haveImage[ " << d << " 1 " << time << " ] ; " << endl;
  }

  
  cout << endl << "// next predicates" << endl;
  for (int t = 1; t <= (time-1); t++) {
    cout << "next[" << t << " " << t+1 << "] ; " ;
    cout << endl;
  }

  return 0;
}
