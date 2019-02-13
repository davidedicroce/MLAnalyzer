// -*- C++ -*-
//
// Package:    MLAnalyzer/SCRegressor
// Class:      SCRegressor
// 
/**\class SCRegressor SCRegressor.cc MLAnalyzer/SCRegressor/plugins/SCRegressor.cc

Description: [one line class summary]

Implementation:
[Notes on implementation]
*/
//
// Original Author:  Michael Andrews
//         Created:  Mon, 17 Jul 2017 15:59:54 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/EcalDetId/interface/EcalTrigTowerDetId.h"
#include "Geometry/CaloTopology/interface/EcalBarrelTopology.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"

#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h" // reco::PhotonCollection defined here

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TTree.h"
#include "TStyle.h"
#include "TMath.h"
#include "TProfile2D.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/deltaPhi.h"

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<> and also remove the line from
// constructor "usesResource("TFileService");"
// This will improve performance in multithreaded jobs.

class SCRegressor : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
  public:
    explicit SCRegressor(const edm::ParameterSet&);
    ~SCRegressor();

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


  private:
    virtual void beginJob() override;
    virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
    virtual void endJob() override;

    // ----------member data ---------------------------
    //edm::EDGetTokenT<edm::View<reco::GsfElectron>> electronCollectionT_;
    edm::EDGetTokenT<reco::GsfElectronCollection> electronCollectionT_;
    edm::EDGetTokenT<reco::PhotonCollection> photonCollectionT_;
    edm::EDGetTokenT<EcalRecHitCollection> EBRecHitCollectionT_;
    edm::EDGetTokenT<reco::GenParticleCollection> genParticleCollectionT_;
    edm::EDGetTokenT<reco::GenJetCollection> genJetCollectionT_;

    static const int nPhotons = 2;
    //static const int nPhotons = 1;
    static const int crop_size = 32;
    //static const bool debug = true;
    static const bool debug = false;

    //TH1D * histo; 
    TH2D * hEB_energy; 
    TH2D * hSC_energy[nPhotons]; 
    TH2D * hSC_time[nPhotons]; 
    TH1F * hSC_mass; 
    TH1F * hDR; 
    TH1F * hdEta; 
    TH1F * hdPhi; 
    TH3F * hdPhidEta; 
    TH1F * hPt; 

    TTree* RHTree;

    float eventId_;
    std::vector<float> vEB_energy_;
    std::vector<float> vEB_SCenergy_;
    std::vector<float> vSC_energy_[nPhotons];
    std::vector<float> vSC_energyT_[nPhotons];
    std::vector<float> vSC_energyZ_[nPhotons];
    std::vector<float> vSC_time_[nPhotons];
    float vPho_pT_[nPhotons];
    float vPho_E_[nPhotons];
    float vPho_eta_[nPhotons];
    float vPho_phi_[nPhotons];
    float vPho_r9_[nPhotons];
    float vPho_sieie_[nPhotons];
    float vSC_mass_[nPhotons];
    float vSC_DR_[nPhotons];
    float vSC_pT_[nPhotons];

    float getEMJetMass( reco::GenJetRef, const GlobalPoint& );
    int nTotal, nPassed;

    //TProfile2D * hnPho;
    TH2F * hnPho;
    TH2F * hnPhoGt2;
    TH1F * hdR_nPhoGt2;
    TH2F * hdPhidEta_nPhoGt2;
    TProfile2D * hdPhidEta_jphoPt_o_iphoPt;
    TH1F * hjphoPt_o_iphoPt;

};

//
// constants, enums and typedefs
//
static const float zs = 0.;

//
// static data member definitions
//

//
// constructors and destructor
//
SCRegressor::SCRegressor(const edm::ParameterSet& iConfig)
{
  //EBRecHitCollectionT_ = consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("EBRecHitCollection"));
  //electronCollectionT_ = consumes<edm::View<reco::GsfElectron>>(iConfig.getParameter<edm::InputTag>("gsfElectronCollection"));
  electronCollectionT_ = consumes<reco::GsfElectronCollection>(iConfig.getParameter<edm::InputTag>("gsfElectronCollection"));
  photonCollectionT_ = consumes<reco::PhotonCollection>(iConfig.getParameter<edm::InputTag>("gedPhotonCollection"));
  EBRecHitCollectionT_ = consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("reducedEBRecHitCollection"));
  genParticleCollectionT_ = consumes<reco::GenParticleCollection>(iConfig.getParameter<edm::InputTag>("genParticleCollection"));
  genJetCollectionT_ = consumes<reco::GenJetCollection>(iConfig.getParameter<edm::InputTag>("genJetCollection"));

  //now do what ever initialization is needed
  usesResource("TFileService");
  edm::Service<TFileService> fs;

  // Histograms
  char hname[50], htitle[50];
  // EB rechits
  hEB_energy = fs->make<TH2D>("EB_energy", "E(i#phi,i#eta);i#phi;i#eta",
      EBDetId::MAX_IPHI  , EBDetId::MIN_IPHI-1, EBDetId::MAX_IPHI,
      2*EBDetId::MAX_IETA,-EBDetId::MAX_IETA,   EBDetId::MAX_IETA );
  // EB SCs
  for(int iPho (0); iPho < nPhotons; iPho++) {
    sprintf(hname, "SC_energy%d",iPho);
    sprintf(htitle,"E(i#phi,i#eta);iphi;ieta");
    hSC_energy[iPho] = fs->make<TH2D>(hname, htitle,
        crop_size, 0, crop_size,
        crop_size, 0, crop_size );
    sprintf(hname, "SC_time%d",iPho);
    sprintf(htitle,"t(i#phi,i#eta);iphi;ieta");
    hSC_time[iPho] = fs->make<TH2D>(hname, htitle,
        crop_size, 0, crop_size,
        crop_size, 0, crop_size );
  }
  hSC_mass = fs->make<TH1F>("SC_mass", "m_{SC};m_{SC}",50, 0., 0.5);
  hDR = fs->make<TH1F>("DR_seed_subJet", "#DeltaR(seed,subJet);#DeltaR",50, 0., 50.*0.0174);
  hdEta = fs->make<TH1F>("dEta_seed_subJet", "#Delta#eta(seed,subJet);#Delta#eta",50, 0., 50.*0.0174);
  hdPhi = fs->make<TH1F>("dPhi_seed_subJet", "#Delta#phi(seed,subJet);#Delta#phi",50, 0., 50.*0.0174);
  hdPhidEta = fs->make<TH3F>("dPhidEta_GG", "#Delta(#phi,#eta,m);#Delta#phi(#gamma,#gamma);#Delta#eta(#gamma,#gamma);m",6, 0., 6.*0.0174, 6, 0., 6.*0.0174, 16., 0.,1.6);
  hPt = fs->make<TH1F>("Pt", "Pt", 65, 30., 160.);
  // Output Tree
  RHTree = fs->make<TTree>("RHTree", "RecHit tree");
  RHTree->Branch("eventId",      &eventId_);
  RHTree->Branch("EB_energy",    &vEB_energy_);
  //RHTree->Branch("EB_SCenergy",  &vSC_energy_);
  for(int iPho (0); iPho < nPhotons; iPho++) {
    sprintf(hname, "SC_energy%d",iPho);
    RHTree->Branch(hname,          &vSC_energy_[iPho]);
    sprintf(hname, "SC_energyT%d",iPho);
    RHTree->Branch(hname,          &vSC_energyT_[iPho]);
    sprintf(hname, "SC_energyZ%d",iPho);
    RHTree->Branch(hname,          &vSC_energyZ_[iPho]);
    sprintf(hname, "SC_time%d",iPho);
    RHTree->Branch(hname,          &vSC_time_[iPho]);
    sprintf(hname, "SC_mass%d",iPho);
    RHTree->Branch(hname,          &vSC_mass_[iPho]);
    sprintf(hname, "SC_DR%d",iPho);
    RHTree->Branch(hname,          &vSC_DR_[iPho]);
    sprintf(hname, "SC_pT%d",iPho);
    RHTree->Branch(hname,          &vSC_pT_[iPho]);
    sprintf(hname, "pho_pT%d",iPho);
    RHTree->Branch(hname,      &vPho_pT_[iPho]);
    sprintf(hname, "pho_E%d",iPho);
    RHTree->Branch(hname,      &vPho_E_[iPho]);
    sprintf(hname, "pho_eta%d",iPho);
    RHTree->Branch(hname,      &vPho_eta_[iPho]);
    sprintf(hname, "pho_phi%d",iPho);
    RHTree->Branch(hname,      &vPho_phi_[iPho]);
    sprintf(hname, "pho_r9%d",iPho);
    RHTree->Branch(hname,      &vPho_r9_[iPho]);
    sprintf(hname, "pho_sieie%d",iPho);
    RHTree->Branch(hname,      &vPho_sieie_[iPho]);
  }

  //hnPho = fs->make<TProfile2D>("nPho", "N(m_{#pi},p_{T,#pi})_{reco};m_{#pi^{0}};p_{T,#pi^0}",
      //8, 0., 1.6, 8, 15., 100.);
  hnPho = fs->make<TH2F>("nPho", "N(m_{#pi},p_{T,#pi})_{reco};m_{#pi^{0}};p_{T,#pi^0}",
      16, 0., 1.6, 17, 15., 100.);
  hnPhoGt2 = fs->make<TH2F>("nPhoGt2", "N_{#gamma #geq 2}(m_{#pi},p_{T,#pi})_{reco};m_{#pi^{0}};p_{T,#pi^0}",
      8, 0., 1.6, 8, 15., 100.);
  hdR_nPhoGt2 = fs->make<TH1F>("dR_nPho_gt_2", "#DeltaR(#gamma,#Gamma)_{reco};#DeltaR",
      24, 0., 6.*0.0174);
  hdPhidEta_nPhoGt2 = fs->make<TH2F>("dPhidEta_nPho_gt_2", "N_{#gamma,reco} #geq 2};#Delta#phi(#gamma,#Gamma);#Delta#eta(#gamma,#Gamma)",
      6, 0., 6.*0.0174, 6, 0., 6.*0.0174);
  hdPhidEta_jphoPt_o_iphoPt = fs->make<TProfile2D>("dPhidEta_jphoPt_o_iphoPt", "p_{T,#gamma} / p_{T,#Gamma};#Delta#phi(#gamma,#Gamma);#Delta#eta(#gamma,#Gamma)",
      6, 0., 6.*0.0174, 6, 0., 6.*0.0174);
  hjphoPt_o_iphoPt = fs->make<TH1F>("jphoPt_o_iphoPt", "p_{T,#gamma} / p_{T,#Gamma};f",
      25, 0., 1.);
}


SCRegressor::~SCRegressor()
{

  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//
const math::XYZTLorentzVector getFinalP4( const reco::Candidate* phoCand ) {
  if ( phoCand->status() == 1 ) {
    return phoCand->p4();
  } else {
    if ( phoCand->numberOfDaughters() == 1 )
      return getFinalP4( phoCand->daughter(0) );
    else
      return phoCand->p4();
  }
}

float SCRegressor::getEMJetMass( reco::GenJetRef iJet, const GlobalPoint & seed )
{
    unsigned int nConstituents = iJet->getGenConstituents().size();
    //const std::vector <const reco::GenParticle*> jetConstituents = iJet->getGenConstituents();
    if ( debug ) std::cout << " >> nConst:" << nConstituents << " eta:" << iJet->eta() << " phi:" << iJet->phi() << std::endl;
    if ( debug ) std::cout << " >> seed eta:" << seed.eta() << " phi:" << seed.phi() << std::endl;

    int nSelConst = 0;
    float dR, dEta, dPhi;
    //math::PtEtaPhiELorentzVectorD jetP4;
    math::XYZTLorentzVector jetP4;
    for ( unsigned int j = 0; j < nConstituents; j++ ) {
      const reco::GenParticle* subJet = iJet->getGenConstituent( j );
      if ( debug ) {
        std::cout << " >> pdgId:" << subJet->pdgId() << " status:" << subJet->status()
        << " pT:" << subJet->pt() << " eta:" << subJet->eta() << " phi: " << subJet->phi() << " E:" << subJet->energy();
        std::cout << " moth:" << subJet->mother()->pdgId();
        std::cout << std::endl;
      }
      if ( std::abs(subJet->pdgId()) != 22 && std::abs(subJet->pdgId()) != 11 ) continue;
      if ( subJet->status() != 1 ) continue;
      if ( subJet->mother()->pdgId() != 111 && subJet->mother()->pdgId() != 35 ) continue;
      //dR = reco::deltaR( seed.eta(),seed.phi(), subJet->eta(),subJet->phi() );
      //dPhi = reco::deltaPhi( seed.phi(), subJet->phi() );
      //dEta = std::abs( seed.eta() - subJet->eta() );
      dR = reco::deltaR( iJet->eta(),iJet->phi(), subJet->eta(),subJet->phi() );
      dPhi = reco::deltaPhi( iJet->phi(), subJet->phi() );
      dEta = std::abs( iJet->eta() - subJet->eta() );
      //if ( dPhi > TMath::Pi() ) dPhi =- TMath::Pi();
      hDR->Fill( dR );
      hdEta->Fill( dEta );
      hdPhi->Fill( dPhi );
      if ( debug ) std::cout << " >> dR:" << dR << " dEta:" << dEta << " dPhi:" << dPhi << std::endl;
      if ( dR > (6*0.0174) ) continue;
      //if ( dR > (4*0.0174) ) continue;
      jetP4 += subJet->p4();
      nSelConst++;
    }
    if ( nSelConst == 0 ) return 0.;
    if ( jetP4.mass() < 1.e-3 ) return 0.;
    //std::cout << " EMJetMass:" << jetP4.mass() << std::endl;
    //std::cout << " >> pT:" << iJet->pt() << " eta:" << iJet->eta() << " phi: " << iJet->phi() << " E:" << iJet->energy() << std::endl;
    return jetP4.mass();
}

// ------------ method called for each event  ------------
void
SCRegressor::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  nTotal++;
  using namespace edm;

  edm::Handle<EcalRecHitCollection> EBRecHitsH;
  iEvent.getByToken(EBRecHitCollectionT_, EBRecHitsH);
  edm::Handle<reco::PhotonCollection> photons;
  iEvent.getByToken(photonCollectionT_, photons);
  edm::Handle<reco::GenParticleCollection> genParticles;
  iEvent.getByToken(genParticleCollectionT_, genParticles);
  // Provides access to global cell position and coordinates below
  edm::ESHandle<CaloGeometry> caloGeomH;
  iSetup.get<CaloGeometryRecord>().get(caloGeomH);
  const CaloGeometry* caloGeom = caloGeomH.product();
  if ( debug ) std::cout << " >> PhoCol.size: " << photons->size() << std::endl;

  for(int iPho (0); iPho < nPhotons; iPho++) {
    vSC_energy_[iPho].assign(crop_size*crop_size,0.);
    vSC_energyT_[iPho].assign(crop_size*crop_size,0.);
    vSC_energyZ_[iPho].assign(crop_size*crop_size,0.);
    vSC_time_[iPho].assign(crop_size*crop_size,0.);
  }
  vEB_SCenergy_.assign(EBDetId::kSizeForDenseIndexing,0.);

  int iphi_, ieta_;

  std::vector<int> vGenPi0Idxs_;
  float dEta, dPhi, dR, mPi0;

  // ____________ Gen-level studies ________________ //
  int nPi0 = 0;
  //std::vector<math::PtEtaPhiELorentzVectorD> vPhoPairs[nPhotons];
  std::vector<math::XYZTLorentzVector> vPhoPairs[nPhotons];
  std::map<unsigned int, std::vector<unsigned int>> mGenPi0_RecoPho;
  for ( unsigned int iG = 0; iG < genParticles->size(); iG++ ) {

    reco::GenParticleRef iGen( genParticles, iG );
    // ID cuts
    if ( debug ) std::cout << " >> pdgId:"<< iGen->pdgId() << " status:" << iGen->status() << " nDaughters:" << iGen->numberOfDaughters() << std::endl;
    if ( std::abs(iGen->pdgId()) != 111 ) continue;
    if ( debug ) std::cout << " >> pdgId:111 nDaughters:" << iGen->numberOfDaughters() << std::endl;
    if ( iGen->numberOfDaughters() != 2 ) continue;
    //if ( iGen->mass() > 0.4 ) continue;
    dR = reco::deltaR( iGen->daughter(0)->eta(),iGen->daughter(0)->phi(), iGen->daughter(1)->eta(),iGen->daughter(1)->phi() );
    if ( dR > 5*.0174 ) continue;

    vGenPi0Idxs_.push_back( iG );
    //mGenPi0_GenPho.insert( std::pair<unsigned int, vector<int>>(iG, vector<int>()) );
    mGenPi0_RecoPho.insert( std::pair<unsigned int, std::vector<unsigned int>>(iG, std::vector<unsigned int>()) );

    for ( unsigned int iD = 0; iD < iGen->numberOfDaughters(); iD++ ) {
      const reco::Candidate* phoCand = iGen->daughter(iD);
      vPhoPairs[nPi0].push_back( getFinalP4(phoCand) );
    }
    nPi0++;

  } // genParticle loop: count good photons
  if ( nPi0 != nPhotons ) return;

  ////////// Apply selection and get coordinates of shower max //////////
  //float ptCut = 45., etaCut = 1.44;
  float ptCut = 15., etaCut = 1.44;

  // Get reco (pT>5GeV) photons associated to pi0
  float minDR = 100.;
  int minDR_idx = -1;
  // Loop over pi0s
  for ( auto& mG : mGenPi0_RecoPho ) {

    reco::GenParticleRef iGen( genParticles, mG.first );

    if ( debug ) std::cout << " >> pi0[" << mG.first << "]"<< std::endl;
    // Loop over photons from pi0
    for ( unsigned int iD = 0; iD < iGen->numberOfDaughters(); iD++ ) {

      const reco::Candidate* iGenPho = iGen->daughter(iD);

      // Loop over reco photon collection
      minDR = 100.;
      minDR_idx = -1;
      if ( debug ) std::cout << "  >> iD[" << iD << "]" << std::endl;
      for ( unsigned int iP = 0; iP < photons->size(); iP++ ) {

        reco::PhotonRef iPho( photons, iP );
        if ( iPho->pt() < 5. ) continue;
        dR = reco::deltaR( iPho->eta(),iPho->phi(), iGenPho->eta(),iGenPho->phi() );
        if ( dR > minDR ) continue;

        minDR = dR;
        minDR_idx = iP;
        if ( debug ) std::cout << "   >> minDR_idx:" << minDR_idx << " " << minDR << std::endl;

      } // reco photons
      if ( minDR > 0.04 ) continue;
      //mG[mG.first].push_back( minDR_idx );
      mG.second.push_back( minDR_idx );
      if ( debug ) std::cout << "   >> !minDR_idx:" << minDR_idx << std::endl;

    } // gen photons 
    dR = reco::deltaR( iGen->daughter(0)->eta(),iGen->daughter(0)->phi(), iGen->daughter(1)->eta(),iGen->daughter(1)->phi() );
    if ( debug ) std::cout << "   >> gen dR:" << dR << std::endl;

  } // gen pi0s

  // Ensure only 1 reco photon associated to each gen pi0
  std::vector<int> vRecoPhoIdxs_;
  for ( auto const& mG : mGenPi0_RecoPho ) {
    if ( debug ) std::cout << " >> pi0[" << mG.first << "] size:" << mG.second.size() << std::endl; 
    // Possibilities:
    // 1) pho_gen1: unmatched, pho_gen2: unmatched => both reco pho failed pT cut or dR matching, reject
    // 2) pho_gen1: reco-matched1, pho_gen1: unmatched => one reco pho failed pT cut or dR matching, can accept
    // 3) pho_gen1: reco-matched1, pho_gen2: reco-matched2
    //    3a) reco-matched1 == reco-matched2 => merged, accept
    //    3b) reco-matched1 != reco-matched2 => resolved, reject
    if ( mG.second.size() == 0 ) continue;
    if ( mG.second.size() == 2 && mG.second[0] != mG.second[1] ) continue; // 2 resolved reco photons 
    vRecoPhoIdxs_.push_back( mG.second[0] );
  } 

  // Ensure each of pi0-matched reco photons passes pre-selection
  // NOTE: at this point there is 1-1 correspondence between pi0 and reco pho,
  // so suffices to check there are as many pre-selected phos as pi0s
  bool isIso;
  std::vector<int> vPreselPhoIdxs_;
  for ( unsigned int i = 0; i < vRecoPhoIdxs_.size(); i++ ) {

    reco::PhotonRef iPho( photons, vRecoPhoIdxs_[i] );

    if ( iPho->pt() < ptCut ) continue;
    if ( std::abs(iPho->eta()) > etaCut ) continue;
    if ( debug ) std::cout << " >> pT: " << iPho->pt() << " eta: " << iPho->eta() << std::endl;
    if ( iPho->r9() < 0.5 ) continue;
    if ( iPho->hadTowOverEm() > 0.07 ) continue;
    if ( iPho->full5x5_sigmaIetaIeta() > 0.0105 ) continue;
    if ( iPho->hasPixelSeed() == true ) continue;

    // Ensure pre-sel photons are isolated 
    isIso = true;
    for ( unsigned int j = 0; j < vRecoPhoIdxs_.size(); j++ ) {

      if ( i == j ) continue;
      reco::PhotonRef jPho( photons, vRecoPhoIdxs_[j] ); 
      dR = reco::deltaR( iPho->eta(),iPho->phi(), jPho->eta(),jPho->phi() );
      if ( debug ) std::cout << "   >> reco dR:" << dR << std::endl;
      if ( dR > 12*.0174 ) continue;
      isIso = false;
      break;

    } // reco photon j
    if ( !isIso ) continue;

    vPreselPhoIdxs_.push_back( vRecoPhoIdxs_[i] );

  } // reco photon i

  if ( vPreselPhoIdxs_.size() != nPhotons ) return;
  //TODO: allow variable number of pre-selected photons to be saved per event

  /*
  // Loop over pre-selected photons
  std::map<const reco::Candidate*, unsigned int> genRecoPhoMap;
  for ( unsigned int i = 0; i < vPreselPhoIdxs_.size(); i++ ) {

    reco::PhotonRef iPho( photons, vPreselPhoIdxs_[i] );

    // Ensure only 1 pre-sel photon exists per gen pi0
    for ( unsigned int iG = 0; iG < genParticles->size(); iG++ ) {

      // Loop over gen photons from pi0
      reco::GenParticleRef iGen( genParticles, iG );
      if ( std::abs(iGen->pdgId()) != 22 ) continue;
      if ( iGen->status() != 1 ) continue; // NOT the same as Pythia status
      if ( iGen->mother()->pdgId() != 111 ) continue;

      // Declare match to the pre-sel photon if within dR cone 
      dR = reco::deltaR( iPho->eta(),iPho->phi(), iGen->eta(),iGen->phi() );
      if ( dR > 0.04 ) continue;
      //std::cout << " >> gen pho: " << iGen->eta() << ", " << iGen->phi() << ": " << iP <<std::endl;

      // If no pre-sel photon matched to mother pi0, save pairing to dict
      // If another pre-sel photon is matched to same pi0, declare as double-matched and completely remove pairing 
      auto pi0It = genRecoPhoMap.find( iGen->mother() );

      if ( pi0It == genRecoPhoMap.end() ) {
        genRecoPhoMap.insert( std::pair<const reco::Candidate*, unsigned int>(iGen->mother(), vPreselPhoIdxs_[i]) );
        //std::cout << " >> Adding: " << iGen->mother()->eta() << ", " << iGen->mother()->phi() << ": " << iP <<std::endl;
      } else {
        //std::cout << " >> Removing: " << iGen->mother()->eta() << ", " << iGen->mother()->phi() << ": " << iP <<std::endl;
        genRecoPhoMap.erase( pi0It );
      } 

      if ( debug ) std::cout << " >> GenPhoMatch | " << " pt:" << iGen->pt() << " dR: " << dR << std::endl;
      break;

    } // genParticles
  } // good photons

  // dR studies
  for ( auto const& iPhoMap : genRecoPhoMap ) {

    reco::PhotonRef iPho( photons, iPhoMap.second );
    //std::cout << " >> sel pho: " << iPhoMap.second << std::endl;

    int nPhoInDR = 1;
    for ( unsigned int j = 0; j < photons->size(); j++ ) {
      if ( iPhoMap.second == j ) continue;
      reco::PhotonRef iPhoCone( photons, j );
      dR = reco::deltaR( iPho->eta(),iPho->phi(), iPhoCone->eta(),iPhoCone->phi() );
      dEta = std::abs( iPho->eta() - iPhoCone->eta() );
      dPhi = reco::deltaPhi( iPho->phi(), iPhoCone->phi() );
      if ( dR > 0.0174*5 ) continue;
      nPhoInDR++;
      hdPhidEta_nPhoGt2->Fill( dPhi, dEta );
      hdPhidEta_jphoPt_o_iphoPt->Fill( dPhi, dEta, iPhoCone->pt() / iPho->pt() ); 
      hdR_nPhoGt2->Fill( dR );
      hjphoPt_o_iphoPt->Fill( iPhoCone->pt() / iPho->pt() );
      //std::cout << " >> DR:" << dR << " pT:" << iPho->pt() << std::endl;
    }
    //std::cout << " >> nPhoInDR:" << nPhoInDR << std::endl;
    float mPi0gen_ = (vPhoPairs[0][0] + vPhoPairs[0][1]).mass();
    reco::GenParticleRef iGen( genParticles, vGenPi0Idxs_[0] );
    float ptPi0gen_ = iGen->pt(); 
    //std::cout << " >> nPhoInDR:" << nPhoInDR << " m:" << mPi0gen_ << " pt:" << ptPi0gen_ << std::endl;

    hnPho->Fill( mPi0gen_, ptPi0gen_, 1.*nPhoInDR );
    if ( nPhoInDR > 1 ) hnPhoGt2->Fill( mPi0gen_, ptPi0gen_);
    if ( nPhoInDR < 2 ) {
      hdR_nPhoGt2->Fill( 0. );
      hjphoPt_o_iphoPt->Fill( 0. );
    }

  } // dR studies
  */

  // Get coordinates of photon supercluster seed 
  int nPho = 0;
  int iphi_Emax, ieta_Emax;
  float Emax;
  GlobalPoint pos_Emax;
  std::vector<GlobalPoint> vPos_Emax;
  std::vector<int> vRegressPhoIdxs_;
  std::vector<float> vIphi_Emax;
  std::vector<float> vIeta_Emax;
  for ( unsigned int i = 0; i < vPreselPhoIdxs_.size(); i++ ) {

    reco::PhotonRef iPho( photons, vPreselPhoIdxs_[i] );

    // Get underlying super cluster
    reco::SuperClusterRef const& iSC = iPho->superCluster();
    //EcalRecHitCollection::const_iterator iRHit_( EBRecHitsH->find(iSC->seed()->seed()) );
    //std::cout << "Seed E: " << iRHit_->energy() << std::endl;
    std::vector<std::pair<DetId, float>> const& SCHits( iSC->hitsAndFractions() );
    //std::cout << " >> SChits.size: " << SCHits.size() << std::endl;

    // Get Emax crystal
    Emax = 0.;
    iphi_Emax = -1;
    ieta_Emax = -1;

    // Loop over SC hits of photon
    for(unsigned iH(0); iH != SCHits.size(); ++iH) {

      // Get DetId
      if ( SCHits[iH].first.subdetId() != EcalBarrel ) continue;
      EcalRecHitCollection::const_iterator iRHit( EBRecHitsH->find(SCHits[iH].first) );
      if ( iRHit == EBRecHitsH->end() ) continue;

      // Convert coordinates to ordinals
      EBDetId ebId( iRHit->id() );
      ieta_ = ebId.ieta() > 0 ? ebId.ieta()-1 : ebId.ieta(); // [-85,...,-1,1,...,85]
      ieta_ += EBDetId::MAX_IETA; // [0,...,169]
      iphi_ = ebId.iphi()-1; // [0,...,359]

      // Keep coordinates of shower max
      if ( iRHit->energy() > Emax ) {
        Emax = iRHit->energy();
        iphi_Emax = iphi_;
        ieta_Emax = ieta_;
        pos_Emax = caloGeom->getPosition(ebId);
      }
      //std::cout << " >> " << iH << ": iphi_,ieta_,E: " << iphi_ << ", " << ieta_ << ", " << iRHit->energy() << std::endl; 
    } // SC hits

    // Apply selection on position of shower seed
    //std::cout << " >> Found: iphi_Emax,ieta_Emax: " << iphi_Emax << ", " << ieta_Emax << std::endl;
    if ( Emax <= zs ) continue;
    if ( ieta_Emax > 169 - 15 || ieta_Emax < 15 ) continue;
    vRegressPhoIdxs_.push_back( vPreselPhoIdxs_[i] );
    vIphi_Emax.push_back( iphi_Emax );
    vIeta_Emax.push_back( ieta_Emax );
    vPos_Emax.push_back( pos_Emax );
    //std::cout << " >> Found: iphi_Emax,ieta_Emax: " << iphi_Emax << ", " << ieta_Emax << std::endl;
    nPho++;

  } // Photons

  // Enforce selection
  //std::cout << " >> nPho: " << nPho << std::endl;
  if ( nPho != nPhotons ) return;
  if ( debug ) std::cout << " >> Passed selection. " << std::endl;

  ////////// Store each shower crop //////////
  for ( unsigned int i = 0; i < nPhotons; i++ ) {
    reco::PhotonRef iPho( photons, vRegressPhoIdxs_[i] );
    // Fill branch arrays
    vPho_pT_[i] = iPho->pt();
    vPho_E_[i] = iPho->energy();
    vPho_eta_[i] = iPho->eta();
    vPho_phi_[i] = iPho->phi();
    //std::cout << "r9: " << iPho->r9() << std::endl;
    vPho_r9_[i] = iPho->r9();
    vPho_sieie_[i] = iPho->full5x5_sigmaIetaIeta(); 
  } // photons
  for ( int i = 0; i < nPi0; i++ ) {
    mPi0 = (vPhoPairs[i][0] + vPhoPairs[i][1]).mass();
    dR = reco::deltaR( vPhoPairs[i][0].eta(),vPhoPairs[i][0].phi(), vPhoPairs[i][1].eta(),vPhoPairs[i][1].phi() );
    dEta = std::abs( vPhoPairs[i][0].eta() - vPhoPairs[i][1].eta() );
    dPhi = reco::deltaPhi( vPhoPairs[i][0].phi(), vPhoPairs[i][1].phi() );
    if ( debug ) std::cout << " >> m0:" << mPi0 << " dR:" << dR << " dPhi:" << dPhi << std::endl;
    reco::GenParticleRef iGen( genParticles, vGenPi0Idxs_[i] );
    vSC_DR_[i] = dR;
    vSC_pT_[i] = iGen->pt(); 
    vSC_mass_[i] = mPi0;
    hPt->Fill( iGen->pt() );
    hdPhidEta->Fill( dPhi, dEta, mPi0 );
    hnPho->Fill( mPi0, iGen->pt() );
  }
  for ( int i = 0; i < nPhotons; i++ ) {
    //std::cout << "SC mass " << vSC_mass_[i] << std::endl;
    hSC_mass->Fill( vSC_mass_[i] );
  }

  int idx;
  int iphi_shift, ieta_shift;
  int iphi_crop, ieta_crop;
  for(EcalRecHitCollection::const_iterator iRHit = EBRecHitsH->begin();
      iRHit != EBRecHitsH->end();
      ++iRHit) {

    if ( iRHit->energy() < zs ) continue;

    // Convert detector coordinates to ordinals
    EBDetId ebId( iRHit->id() );
    iphi_ = ebId.iphi()-1; // [0,...,359]
    ieta_ = ebId.ieta() > 0 ? ebId.ieta()-1 : ebId.ieta(); // [-85,...,-1,0,...,84]
    ieta_ += EBDetId::MAX_IETA; // [0,...,169]

    for(unsigned iP(0); iP < nPhotons; iP++) {

      iphi_shift = vIphi_Emax[iP] - 15;
      ieta_shift = vIeta_Emax[iP] - 15;
      //std::cout << " >> Storing: iphi_Emax,ieta_Emax: " << vIphi_Emax[iP] << ", " << vIeta_Emax[iP] << std::endl;

      // Convert to [0,...,31][0,...,31]
      ieta_crop = ieta_ - ieta_shift;
      iphi_crop = iphi_ - iphi_shift;
      if ( iphi_crop >= EBDetId::MAX_IPHI ) iphi_crop = iphi_crop - EBDetId::MAX_IPHI; // get wrap-around hits

      if ( ieta_crop < 0 || ieta_crop > crop_size-1 ) continue;
      if ( iphi_crop < 0 || iphi_crop > crop_size-1 ) continue;
      
      // Convert to [0,...,32*32-1] 
      idx = ieta_crop*crop_size + iphi_crop;

      // Cell geometry provides access to (rho,eta,phi) coordinates of cell center
      //auto cell = caloGeom->getGeometry(ebId);
      auto pos = caloGeom->getPosition(ebId);
      
      // Fill branch arrays 
      vSC_energy_[iP][idx] = iRHit->energy();
      //vSC_energyT_[iP][idx] = iRHit->energy()/TMath::CosH(vPho_eta_[iP]);
      vSC_energyT_[iP][idx] = iRHit->energy()/TMath::CosH(pos.eta());
      vSC_energyZ_[iP][idx] = iRHit->energy()*std::abs(TMath::TanH(pos.eta()));
      vSC_time_[iP][idx] = iRHit->time();
      vEB_SCenergy_[ebId.hashedIndex()] = iRHit->energy();
      //std::cout << " >> " << iP << ": iphi_,ieta_,E: " << iphi_crop << ", " << ieta_crop << ", " << iRHit->energy() << std::endl; 

      // Fill histograms to monitor cumulative distributions
      hSC_energy[iP]->Fill( iphi_crop,ieta_crop,iRHit->energy() );
      hSC_time[iP]->Fill( iphi_crop,ieta_crop,iRHit->time() );

    } // EB rechits
  } // photons

  // Fill full EB for comparison
  vEB_energy_.assign(EBDetId::kSizeForDenseIndexing,0.);
  for(EcalRecHitCollection::const_iterator iRHit = EBRecHitsH->begin();
      iRHit != EBRecHitsH->end();
      ++iRHit) {

    // Get detector id and convert to histogram-friendly coordinates
    EBDetId ebId( iRHit->id() );
    iphi_ = ebId.iphi()-1;
    ieta_ = ebId.ieta() > 0 ? ebId.ieta()-1 : ebId.ieta();
    //std::cout << "ECAL | (ieta,iphi): (" << ebId.ieta() << "," << ebId.iphi() << ")" <<std::endl;

    // Fill some histograms to monitor distributions
    // These will contain *cumulative* statistics and as such
    // should be used for monitoring purposes only
    hEB_energy->Fill( iphi_,ieta_,iRHit->energy() );

    // Fill branch arrays
    idx = ebId.hashedIndex(); // (ieta_+EBDetId::MAX_IETA)*EBDetId::MAX_IPHI + iphi_
    vEB_energy_[idx] = iRHit->energy();
  } // EB rechits

  eventId_ = iEvent.id().event();
  nPassed++;

  RHTree->Fill();

#ifdef THIS_IS_AN_EVENT_EXAMPLE
  Handle<ExampleData> pIn;
  iEvent.getByLabel("example",pIn);
#endif

#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  ESHandle<SetupData> pSetup;
  iSetup.get<SetupRecord>().get(pSetup);
#endif
}


// ------------ method called once each job just before starting event loop  ------------
void 
SCRegressor::beginJob()
{
  nTotal = 0;
  nPassed = 0;
}

// ------------ method called once each job just after ending the event loop  ------------
void 
SCRegressor::endJob() 
{
  std::cout << " selected: " << nPassed << "/" << nTotal << std::endl;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
SCRegressor::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks","ctfWithMaterialTracks");
  //descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(SCRegressor);