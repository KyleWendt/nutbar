
#include <string>
#include <set>

#include "JBasis.hh"

//#define VERBOSE true

using namespace std;

JBasis::JBasis()
{}

JBasis::JBasis(int j2, int m2)
 : J2(j2), M2(m2)
{}

JBasis::JBasis( string sps_file,  vector<string> proton_files, vector<string> neutron_files, int j2, int m2 )
 : J2(j2), M2(m2)
{
  SetupBasis( sps_file, proton_files, neutron_files );
}

/*

void JBasis::SetupBasis( string sps_file,  vector<string> A_files, vector<string> B_files  )
{
//  #ifdef VERBOSE
    cout << "JBasis::SetupBasis -- begin. sps_file = " << sps_file << endl;
//  #endif
  NuBasis nubasis;
  nubasis.ReadSPS( sps_file);
//  #ifdef VERBOSE
    cout << "JBasis::SetupBasis -- done reading sps_file " << endl;
//  #endif

  vector<int> offsets_a;
  vector<int> offsets_b;
  cout << "About to call SetUpJMState_ab" << endl;
  SetUpJMState_ab( sps_file, A_files, jmstates_a, offsets_a);
  cout << "About to call SetUpJMState_ab" << endl;
  SetUpJMState_ab( sps_file, B_files, jmstates_b, offsets_b);
  
  cout << "done setting up A and B states. A dimension = " << jmstates_a.size() << "  B dimension = " << jmstates_b.size() << endl;

  for (int ind_Afile=0; ind_Afile<A_files.size(); ++ind_Afile)
  {
   int ia_min = offsets_a[ind_Afile];
   int ia_max = (ind_Afile<A_files.size()-1) ? offsets_a[ind_Afile+1] : A_files.size();
   for (int ind_Bfile=0; ind_Bfile<B_files.size(); ++ind_Bfile)
   {

     int ib_min = offsets_b[ind_Bfile];
     int ib_max = (ind_Bfile<B_files.size()-1) ? offsets_b[ind_Bfile+1] : B_files.size();
     for (int i_b=ib_min; i_b<ib_max; ++i_b)
     {
      int JB = jmstates_b[i_b].J2;
      for (int i_a=ia_min; i_a<ia_max; ++i_a)
      {
        int JA = jmstates_a[i_a].J2;
        if (JA+JB<J2 or abs(JA-JB)>J2) continue;
        basis_states.emplace_back( array<int,4>({ i_a, i_b, J2, M2 }) );
      }
     }
   }
  }
 cout << "Done setting up Jbasis. Dimension = " << basis_states.size() << endl;
}


*/

void JBasis::SetupBasis( string sps_file,  vector<string> A_files, vector<string> B_files  )
{
  #ifdef VERBOSE
    cout << "JBasis::SetupBasis -- begin. sps_file = " << sps_file << endl;
  #endif
  NuProj nuproj;
  NuBasis nubasis;
  nubasis.ReadSPS( sps_file);
  m_orbits = nubasis.m_orbits;
  #ifdef VERBOSE
    cout << "JBasis::SetupBasis -- done reading sps_file " << endl;
  #endif

  vector<int> offsets_a;
  vector<int> offsets_b;

  SetUpJMState_ab( nubasis, nuproj, A_files, jmstates_a, offsets_a);
  SetUpJMState_ab( nubasis, nuproj, B_files, jmstates_b, offsets_b);
  

  for (int ind_Afile=0; ind_Afile<A_files.size(); ++ind_Afile)
  {
   int ia_min = offsets_a[ind_Afile];
   int ia_max = (ind_Afile<A_files.size()-1) ? offsets_a[ind_Afile+1] : A_files.size();
   for (int ind_Bfile=0; ind_Bfile<B_files.size(); ++ind_Bfile)
   {

     int ib_min = offsets_b[ind_Bfile];
     int ib_max = (ind_Bfile<B_files.size()-1) ? offsets_b[ind_Bfile+1] : B_files.size();
     for (int i_b=ib_min; i_b<ib_max; ++i_b)
     {
      int JB = jmstates_b[i_b].J2;
      for (int i_a=ia_min; i_a<ia_max; ++i_a)
      {
        int JA = jmstates_a[i_a].J2;
        if (JA+JB<J2 or abs(JA-JB)>J2) continue;
        basis_states.emplace_back( array<int,4>({ i_a, i_b, J2, M2 }) );
      }
     }
   }
  }
}

void JBasis::SetUpJMState_ab( NuBasis& nubasis, NuProj& nuproj,  vector<string> filenames, vector<JMState>& jmstates,  vector<int>& offsets)
{
  for (string afile : filenames)
  {
    #ifdef VERBOSE
      cout << "JBasis::SetupBasis -- afile =  " << afile << endl;
     #endif
    nubasis.ReadFile(afile + ".nba");
    nuproj.ReadFile(afile + ".prj");
    int ngood = nuproj.ngood;

    offsets.push_back( jmstates.size() );
    for (int i=0;i<ngood;++i)
    {
      if (nubasis.ibf[nuproj.pindx[i]-1]<1) continue;
      jmstates.emplace_back( nubasis, nuproj, i);
    }
  }
}

JMState JBasis::GetBasisState(size_t index) const
{
  int i_a = basis_states[index][0];
  int i_b = basis_states[index][1];
  int J2 = basis_states[index][2];
  int M2 = basis_states[index][3];
  return TensorProduct( jmstates_a[i_a], jmstates_b[i_b], J2, M2 );
//  return basis_states[index];
}



int JBasis::GetNaiveMschemeDimension() const
{
//  set<key_type> mstates_a, mstates_b;
  map<int,set<uint64_t>> mstates_a, mstates_b;
  for (auto& jst : jmstates_a)
  {
    if (mstates_a.count(jst.M2)<1) mstates_a[jst.M2] = set<uint64_t>();
    for (auto& itm: jst.m_coefs)
    {
      uint64_t k = itm.first.to_ulong();
//      if (mstates_a.count(itm.first)<1) mstates_a.insert(itm.first);
      if (mstates_a[jst.M2].count(k)<1 and jst.M2==3)
      {
        mstates_a[jst.M2].insert(k);
        cout << "Inserted " << itm.first <<  " J,M = (" << jst.J2 <<  " " << jst.M2 << ")   k = " << k << endl;
      }
    }
  }
  for (auto& jst : jmstates_b)
  {
    if (mstates_b.count(jst.M2)<1) mstates_b[jst.M2] = set<uint64_t>();
    for (auto& itm: jst.m_coefs)
    {
      uint64_t k = itm.first.to_ulong();
      if (mstates_b[jst.M2].count(k)<1) mstates_b[jst.M2].insert(k);
//      if (mstates_b.count(itm.first)<1) mstates_b.insert(itm.first);
    }
  }
  int nm = 0;
  cout << "J2,M2 = " << J2 << " " << M2 << endl;
  for (auto& it_a : mstates_a)
  {
    int ma = it_a.first;
    int mb = abs(J2 - ma);
    cout << "ma = " << ma << endl;
    if (mstates_b.count(mb))
    {
      cout << "ma = " << ma << "  mb = " << mb << "  sizes = " << mstates_a[ma].size() << " " << mstates_b[mb].size() << endl;
      nm += (min(ma,mb)+1)*it_a.second.size() * mstates_b[mb].size();
      for ( auto vecb : mstates_b[mb]) cout << bitset<64>(vecb) << endl;
    }
  }
//  cout << "dim a: " << mstates_a.size() << "   dim b: " << mstates_b.size() << "  product = " << mstates_a.size() * mstates_b.size() << endl;
  cout << "dim = " << nm << endl;
  return mstates_a.size() * mstates_b.size();

}

