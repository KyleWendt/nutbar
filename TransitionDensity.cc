#include <cmath>
#include <sstream>
#include <algorithm>


#include "TransitionDensity.hh"

using namespace std;


vector<char> TransitionDensity::an_code = {  '0','1','2','3','4','5','6','7','8','9',
                                             '_','-','c','d','e','f','g','h','i','j',  
                                             'k','l','m','n','o','p','q','r','s','t',  
                                             'u','v','w','x','y','z','A','B','C','D',  
                                             'E','F','G','H','I','J','K','L','M','N',  
                                             'O','P','Q','R','S','T','U','V','W','X',  
                                             '0','1','2','3','4','5','6','7','8','9',  
                                             '_','-','c'};

vector<string> TransitionDensity::periodic_table = {
   "N","h_","he",
       "li","be","b_","c_","n_","o_","f_","ne",
       "na","mg","al","si","p_","s_","cl","ar",
       "k_","ca","sc","ti","v_","cr","mn","fe","co","ni","cu","zn","ga","ge","as","se","br","kr",
       "rb","sr","y_","zr","nb","mo","tc","ru","rh","pd","ag","cd","in","sn","sb","te","i_","xe",
       "cs","ba",
         "la","ce","pr","nd","pm","sm","eu","gd","tb","dy","ho","er","tm","yb",
                 "lu","hf","ta","w_","re","os","ir","pr","au","hg","tl","pb","bi","po","at","rn",
       "fr","ra",
         "ac","th","u_","np","pu","am","cm","bk","cf","es","fm","md","no","lr",
                 "rf","db","sg","bh","hs","mt","ds","rg","cn"};


TransitionDensity::TransitionDensity()
: total_number_levels(0)
{}

TransitionDensity::TransitionDensity(vector<int> jlist)
:total_number_levels(0), Jlist(jlist)
{}




void TransitionDensity::ReadInputInteractive()
{

  cout << endl;
  cout << "Name of of NuShellFiles, (e.g. ne200): ";
  cout.flush();
  cin >> basename;

  cout << "Name of sps file, (e.g. sdpn.sps): ";
  cout.flush();
  cin >> sps_file_name;

  cout << "sps_file_name = " << sps_file_name << endl;



  istringstream iss;
  string line;

  cout << "J*2 values of eigenvectors (separated by space): ";
  cout.flush();
  getline(cin,line);
  getline(cin,line);
  iss.clear();
  iss.str(line);
  int j;
  while( iss >> j ) Jlist.push_back(j);

//  cout << "J values: ";
//  for (auto j : Jlist) cout << j << " ";
//  cout << endl;


  cout << "Number of eigenstates for each J (separated by space, or single value if same for all J): ";
  cout.flush();
  getline(cin,line);
  iss.clear();
  iss.str(line);
  vector<int> spj;
  while( iss >> j ) spj.push_back(j);
  for (size_t i=0;i<Jlist.size();++i)
  {
    if (spj.size() < Jlist.size()) max_states_per_J[ Jlist[i] ] = spj[0];
    else                           max_states_per_J[ Jlist[i] ] = spj[i];
  }

}



void TransitionDensity::ReadInputFromFile(string filename)
{
  ifstream infile(filename);
  istringstream iss;

  const int BUFFERSIZE = 500;
  char line[BUFFERSIZE];


  // basename for *.nba *.prj and *.xvc files
  infile.getline(line, BUFFERSIZE);
  iss.str(line);
  iss >> basename;

  cout << "basename = " << basename << endl;

  // basename for *.nba *.prj and *.xvc files
  infile.getline(line, BUFFERSIZE);
  iss.clear();
  iss.str(line);
  iss >> sps_file_name;

  cout << "sps_file_name = " << sps_file_name << endl;


  // list of 2*J values
  infile.getline(line, BUFFERSIZE);
  iss.clear();
  iss.str(line);
  int j;
  while( iss >> j ) Jlist.push_back(j);

  cout << "J values: ";
  for (auto j : Jlist) cout << j << " ";
  cout << endl;

  // number of states to treat for each J
  infile.getline(line, BUFFERSIZE);
  iss.clear();
  iss.str(line);
  vector<int> spj;
  while( iss >> j ) spj.push_back(j);
  for (size_t i=0;i<Jlist.size();++i)
  {
    if (spj.size() < Jlist.size()) max_states_per_J[ Jlist[i] ] = spj[0];
    else                           max_states_per_J[ Jlist[i] ] = spj[i];
  }




}





//void TransitionDensity::ReadFiles( string spsfile, string abfile_base )
void TransitionDensity::ReadFiles( )
{

  ifstream testread; // for checking if files exist
  ostringstream ostr;
  vector<string> Afiles,Bfiles;
  GetAZFromFileName();
  int nvalence_protons = Z - Zcore;
  int nvalence_neutrons = A-Z - (Acore-Zcore);
  for ( auto j : Jlist ) cout << j << " ";
  cout << endl;
  MJtot = (*min_element(begin(Jlist),end(Jlist)))%2;


  
  
  // Find all the prj and nba files
  int iJ;
  for (int icode : { nvalence_protons, -nvalence_protons, nvalence_neutrons, -nvalence_neutrons  } )
  {
    iJ=0;
    while (true)
    {
      ostr.str("");
      ostr.str().clear();
      ostr << basename << "a" << iJ << "0" << an_code[36 + icode/2];
      testread.open(ostr.str()+".nba");
      if ( testread.fail() ) break;
      if ( find(Afiles.begin(),Afiles.end(), ostr.str() ) == Afiles.end())
        Afiles.push_back( ostr.str() );
      testread.close();
      iJ++;
    }
    iJ=0;
    while (true)
    {
      ostr.str("");
      ostr.str().clear();
      ostr << basename << "b" << iJ << "0" << an_code[36 + icode/2];
      testread.open(ostr.str()+".nba");
      if ( testread.fail() ) break;
      if ( find(Bfiles.begin(),Bfiles.end(), ostr.str() ) == Bfiles.end())
      Bfiles.push_back( ostr.str() );
      testread.close();
      iJ++;
    }
  }

  for (int Jtot : Jlist )
  {
    jbasis_list.emplace_back( JBasis( sps_file_name, Afiles, Bfiles, Jtot, MJtot));
  
  
  // Guess the name of the xvc file
    ostr.str("");
    ostr.clear();
    ostr << basename << an_code[Jtot/2] << an_code[nvalence_protons] << an_code[nvalence_neutrons] << ".xvc";
    string vecfile = ostr.str();
    testread.open(vecfile);
    if ( testread.fail() )
    {
      ostr.str("");
      ostr.clear();
      ostr << basename << an_code[Jtot/2] << an_code[nvalence_neutrons] << an_code[nvalence_protons] << ".xvc";
      vecfile = ostr.str();
      testread.close();
      testread.open(vecfile);
      if ( testread.fail() )
      {
        cout << "ERROR! I cant figure out what the *.xvc file should be. Exiting." << endl;
        cout << "( " << ostr.str() << " ) didnt work. abfile_base = " << basename << endl;
        return ;
      }
    }
    testread.close();
  
    nuvec_list.emplace_back( NuVec(Jtot) );
    nuvec_list.back().ReadFile(vecfile);
  
  }

  m_orbits = jbasis_list[0].nubasis_a.m_orbits;
  Nshell=0;
  for (auto morbit : m_orbits)
  {
    Nshell = max(Nshell, 2*morbit.n + morbit.l2/2);
  }
  Nshell++;

  for (size_t ivec=0; ivec<nuvec_list.size(); ++ivec)
  {
   int imax = nuvec_list[ivec].no_level;
   if ( max_states_per_J.find(nuvec_list[ivec].J2) != max_states_per_J.end() ) imax = max_states_per_J[nuvec_list[ivec].J2];
    blank_vector.push_back(vector<float>(imax, 0.));
    total_number_levels += blank_vector.back().size();
  }

}


void TransitionDensity::CalculateMschemeAmplitudes()
{
  for (size_t ivec=0; ivec<nuvec_list.size(); ++ivec)
  {
   cout << "ivec = " << ivec << endl;
   auto& nuvec = nuvec_list[ivec];
   auto& jbasis = jbasis_list[ivec];
   cout << "no_state = " << nuvec.no_state << endl;
   cout << "no_level = " << nuvec.no_level << endl;
  
   // loop over J-coupled basis states
   for (int istate=0;istate<nuvec.no_state;++istate)
   {

     int imax = nuvec.no_level;
     if ( max_states_per_J.find(nuvec.J2) != max_states_per_J.end() ) imax = max_states_per_J[nuvec.J2];
//     vector<float> level_coefs(nuvec.no_level,0.0);
     vector<float> level_coefs(imax,0.0);
//     for (int ilevel=0;ilevel<nuvec.no_level;++ilevel)
     for (int ilevel=0;ilevel<imax;++ilevel)
     {
       level_coefs[ilevel] = nuvec.coefT[ilevel][istate];
     }
  
     JMState& jmst = jbasis.basis_states[istate];
     for (auto& it_mstate : jmst.m_coefs)
     {
       auto& key = it_mstate.first;
       float& m_coef = it_mstate.second;
       if( amplitudes.find(key) == amplitudes.end() ) amplitudes[key] = blank_vector;
       amplitudes[key][ivec] += m_coef * level_coefs;
     }
   }
  }

}



double TransitionDensity::OBTD(int J_index_i, int eigvec_i, int J_index_f, int eigvec_f, int m_index_a, int m_index_b, int Lambda2 )
{

  int J2i = Jlist[J_index_i];
  int J2f = Jlist[J_index_f];
  
  int j2_a = m_orbits[m_index_a].j2;
  int j2_b = m_orbits[m_index_b].j2;

  if ((J2f+Lambda2 < J2i) or (abs(J2f-Lambda2)>J2i)) return 0;
  
  double clebsch_fi = CG(J2f,0,Lambda2,0,J2i,0);
  if (abs(clebsch_fi)<1e-9) cout << "Warning got zero Clebsch while inverting the Wigner-Eckart theorem" << endl;
  
  double obd = 0;
  while ( m_orbits[m_index_a].mj2 < m_orbits[m_index_b].mj2 ) m_index_a++;
  while ( m_orbits[m_index_b].mj2 < m_orbits[m_index_a].mj2 ) m_index_b++;
  if (m_orbits[m_index_a].j2 != j2_a) return 0;
  if (m_orbits[m_index_b].j2 != j2_b) return 0;
  m_index_a--;
  m_index_b--;
  for ( int im=-min(j2_a,j2_b);im<=min(j2_a,j2_b);im+=2)
  {
    m_index_a++;
    m_index_b++;
    for ( auto& it_amp : amplitudes )
    {
      auto& key = it_amp.first;
      if ( not( (key[0] >> m_index_a)&0x1) ) continue;
      if ( m_index_a != m_index_b and   ( (key[0] >> m_index_b)&0x1) ) continue;
      float amp_i = it_amp.second[J_index_i][eigvec_i];
      if (abs(amp_i)<1e-6) continue;
      auto new_key = key;
      new_key[0] &= ~( 0x1 << (m_index_a));
      new_key[0] |=  ( 0x1 << (m_index_b));
      float amp_f = amplitudes[new_key][J_index_f][eigvec_f];
      float clebsch = CG(j2_a,im,j2_b,-im,Lambda2,0) * (1-(j2_b-im)%4);
      obd += clebsch * amp_i * amp_f;
    }
  }
  
  obd *= sqrt((J2i+1.)/(Lambda2+1)) / clebsch_fi;

  return obd;

}





void TransitionDensity::WriteEGV(string fname)
{

  vector<MschemeOrbit> mscheme_orbits;

  // find all the core orbits
  for (int N=0;N<Nshell;++N) 
  {
    int sumN = (N+1)*(N+2)*(N+3)/3;
    for (int n=0;2*n<=N;++n)
    {
      int l2 = 2*(N-2*n);
      for (int j2=l2+1;j2>=max(0,l2-1);j2-=2)
      {
       for (int mj2=-j2;mj2<=j2;mj2+=2)
       {
        if ( sumN<=Zcore )        mscheme_orbits.emplace_back( MschemeOrbit(n,l2,j2,mj2,+1 ) );
        if ( sumN<=Acore-Zcore )  mscheme_orbits.emplace_back( MschemeOrbit(n,l2,j2,mj2,-1 ) );
       }
      }
    }
  }


  // loop over the valence orbits
  int n_core_orbits = mscheme_orbits.size();
  for (auto& morbit : jbasis_list[0].nubasis_a.m_orbits)
  {
     mscheme_orbits.push_back ( morbit );
  }
  // calculate parity of 0hw configurations
  int N=0;
  for (N=0;(N+1)*(N+2)*(N+3)/3 < Z;++N){};
  int parity_protons = (N%2);
  for (N=0;(N+1)*(N+2)*(N+3)/3 < A-Z;++N){};
  int parity_neutrons = (N%2);
  int parity = 1-2*(( parity_protons*(Z-Zcore) + parity_neutrons*(A-Acore-(Z-Zcore)))%2);
  
  ofstream output(fname);
  
  string interaction_id = "imsrg";
  int Nmax = 0;
  float hw = 20; // this should be irrelevant

  output << left << setw(10) << Z                              << " !  number of protons" << endl;
  output << left << setw(10) << A-Z                            << " !  number of neutrons" << endl;
  output << left << setw(max(10,(int)interaction_id.size()+3)) << interaction_id << " ! interaction id" << endl;
  output << left << setw(10) << hw                             << " ! hbar omega" << endl;
  output << left << setw(10) << Nshell                         << " ! N shells " << endl;
  output << left << setw(10) << mscheme_orbits.size()          << " ! m-scheme orbits " << endl;
  output << left << setw(10) << Nmax                           << " ! Nmax " << endl;
  output << left << setw(10) << amplitudes.size()              << " ! number of basis Slater determinants" << endl;
  output << left << setw(10) << showpos << parity << noshowpos << " ! parity " << endl;
  output << left << setw(10) << MJtot                          << " ! total MJ*2" << endl;
  output << left << setw(10) << total_number_levels            << " ! Number of eigenstates" << endl;
  
  // write out energies, J, T of eigenstates
  for (auto& nuvec : nuvec_list)
  {
   int imax = nuvec.no_level;
   if ( max_states_per_J.find(nuvec.J2) != max_states_per_J.end() ) imax = max_states_per_J[nuvec.J2];
   for (int ilevel=0;ilevel<imax;++ilevel)
   {
     output << right << fixed << setw(12) << setprecision(4) << nuvec.alpha[ilevel] << " " << nuvec.J2/2.0 << " " << 0 << " " << 0.0 << endl;
   }
  }
  
  
  output << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
  output << "!!!  now list mscheme single-particle basis !!!" << endl;
  output << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
  // write single-particle basis in mscheme
  
  
  
  int s = 1;
  for (auto& morbit : mscheme_orbits)
  {
    output << setw(4) << s << " " << setw(3) << morbit.n << " " << setw(3) << morbit.l2 << " " << setw(3) << morbit.j2 << " " << setw(3) << morbit.mj2 << " " << setw(3) << morbit.tz2 << endl;;
    s++;
  }
  
  output << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
  output << "!!!  now list mscheme basis states and amplitudes !!!" << endl;
  output << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
  
  
//  cout << "n_core_orbits = " << n_core_orbits << "  mscheme_orbits.size = " << mscheme_orbits.size() << endl;
  
  // Write m-scheme basis occupations and eigenvector coefficients
  size_t bits_per_word = 8*sizeof(mvec_type);
  for (auto it_amp : amplitudes)
  {
    auto& mvec = it_amp.first;
    for (int i=0;i<n_core_orbits;++i) output << i+1 << " ";
    output << " " ;
    for (size_t i=0;i<mscheme_orbits.size()-n_core_orbits;++i)
    {
      if ( (mvec[i/bits_per_word] >> (i%bits_per_word))&0x1)
          output << n_core_orbits + i+1 << " ";
    }
    output << "   ";
    for (auto& ampvec : it_amp.second)
    {
      for (auto amp : ampvec )
      {
        output << scientific << setw(13) << amp << " ";
      }
    }
    output << endl;
  
  }

}




void TransitionDensity::WriteTRDENS_input(string fname)
{
  ofstream outfile(fname);

  outfile << "T        ! specify which states to take?" << endl;
  outfile << "1   " << total_number_levels << "   ! ki,nki" << endl;
  for (int i=1;i<=total_number_levels;++i) outfile << i << " ";
  outfile << endl;
  outfile << "1   " << total_number_levels << "   ! kf,nkf" << endl;
  for (int i=1;i<=total_number_levels;++i) outfile << i << " ";
  outfile << endl;
  outfile << *max_element(begin(Jlist),end(Jlist)) << "      ! jtotal2max" << endl;
  outfile << "0                   ! irestart" << endl;
  outfile << "2                   ! majortot" << endl;
  outfile << "F                   ! irem_cal" << endl;
  outfile << "F                   ! formfcal" << endl;
  outfile << "F                   ! radial  " << endl;
  outfile << "F                   ! momdist " << endl;
  outfile << "T                   ! twobdcal" << endl;
  outfile << "1                   ! ipn     " << endl;
  outfile << "IMSRG.int_iso                 " << endl;
  outfile << "IMSRG_E2_1b.op_iso            " << endl;
  outfile << "IMSRG_E2_2b.op_iso            " << endl;
  outfile << "F                   ! cluster " << endl;
  outfile << "F                   ! antoine " << endl;
  outfile << "24.d0               ! hbar*Omega for antoine file" << endl;
  outfile << "1                   ! #init states in antoine file" << endl;
  outfile << "0  0 " << endl;
  outfile << "1                   ! #fin states in antoine file" << endl;
  outfile << "0  0 " << endl;
  outfile << "T                   ! mbpt_ncsm " << endl;
  outfile << "F                   ! redstick" << endl;
  outfile << "F                   ! mfd_james" << endl;
  outfile << "F                   ! threebdcal" << endl;
  outfile << "F                   ! NCSMC_kernels" << endl;
  outfile << "3                   ! num_of_interaction_files" << endl;
  outfile << "../vrelnp_rgm.int_H2srg-n3lo2.2_2014 " << endl;
  outfile << "../vrelpp_rgm.int_H2srg-n3lo2.2_2014 " << endl;
  outfile << "../vrelnn_rgm.int_H2srg-n3lo2.2_2014 " << endl;
  outfile << "F                   ! V3Nint " << endl;
  outfile << "14 14 14            ! N1_max,N12_max,N123_max " << endl;
  outfile << "chi2b3b400cD-02cE0098_srg0800ho40C_eMax14_EMax14_hwHO016.me3j_bin" << endl;


  outfile.close();


}




void TransitionDensity::GetAZFromFileName(  )
{
  string trimmed_basename = basename.substr( basename.find_last_of("/")+1 );
  string element = trimmed_basename.substr( 0,2);
  cout << trimmed_basename << " " << element << endl;
  auto el_position = find( periodic_table.begin(),periodic_table.end(), element);
  if (el_position == periodic_table.end())
  {
   cout << "ERROR! : could not find " << element << " in periodic table" << endl;
   return;
  }
  Z = el_position - periodic_table.begin();
  istringstream( trimmed_basename.substr(2,2) ) >> A;

  // now guess at what the core should be, assuming a full major oscillator shell valence space
  for (int N=0;(N+1)*(N+2)*(N+3)/3<Z;++N) Zcore = (N+1)*(N+2)*(N+3)/3; 
  for (int N=0;(N+1)*(N+2)*(N+3)/3<(A-Z);++N) Acore = Zcore + (N+1)*(N+2)*(N+3)/3; 
  cout << "Acore,Zcore = " << Acore << " " << Zcore << endl;
  
}




vector<float> operator*(const float lhs, const vector<float>& rhs)
{
  vector<float> vout = rhs;
  for (size_t i=0;i<vout.size();++i) vout[i] *= lhs;
  return vout;
}

vector<float> operator*(const vector<float>& lhs, const vector<float>& rhs)
{
  vector<float> vout = rhs;
  for (size_t i=0;i<vout.size();++i) vout[i] *= lhs[i];
  return vout;
}

vector<float>& operator+=(vector<float>& lhs, const vector<float>& rhs)
{
  for (size_t i=0;i<lhs.size();++i) lhs[i] += rhs[i];
  return lhs;
}



