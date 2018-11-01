#ifndef RecHitAnalyzer_h
#define RecHitAnalyzer_h
// -*- C++ -*-
//
// Package:    MLAnalyzer/RecHitAnalyzer
// Class:      RecHitAnalyzer
// 
//
// Original Author:  Michael Andrews
//         Created:  Sat, 14 Jan 2017 17:45:54 GMT
//
//


// system include files
#include <memory>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
//#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
//#include "DataFormats/EcalDetId/interface/EcalTrigTowerDetId.h"
//#include "Geometry/CaloTopology/interface/EcalBarrelTopology.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"

#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
//#include "DataFormats/HcalDetId/interface/HcalDetId.h"
//#include "DataFormats/HcalDetId/interface/HcalElectronicsId.h"
//#include "DQM/HcalCommon/interface/Constants.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
//#include "DataFormats/TrackReco/interface/Track.h"
//#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHitFwd.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile2D.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TMath.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h" // reco::PhotonCollection defined here
#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/JetReco/interface/GenJet.h"
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

//class RecHitAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
class RecHitAnalyzer : public edm::EDAnalyzer  {
  public:
    explicit RecHitAnalyzer(const edm::ParameterSet&);
    ~RecHitAnalyzer();

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


  private:
    virtual void beginJob() override;
    virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
    virtual void endJob() override;

    // ----------member data ---------------------------
    edm::InputTag EBRecHitCollectionT_;
    edm::InputTag EBDigiCollectionT_;
    edm::InputTag EERecHitCollectionT_;
    edm::InputTag HBHERecHitCollectionT_;
    edm::InputTag TRKRecHitCollectionT_;
    edm::InputTag genParticleCollectionT_;
    edm::InputTag photonCollectionT_;
    edm::InputTag jetCollectionT_;
    edm::InputTag genJetCollectionT_;
    edm::InputTag trackCollectionT_;
    edm::InputTag pfCandCollectionT_;

    // Diagnostic histograms
    //TH2D * hEB_adc[EcalDataFrame::MAXSAMPLES]; 
    //TH1D * hHBHE_depth; 
    TH1F *h_sel; 
    int runCount[4];
    int nTotal;

    // Main TTree
    TTree* RHTree;

    // Objects used to fill RHTree branches
    //std::vector<float> vEB_adc_[EcalDataFrame::MAXSAMPLES];
    //std::vector<float> vFC_inputs_;
    //math::PtEtaPhiELorentzVectorD vPho_[2];
  
    // Selection and filling functions
    void branchesEvtSel         ( TTree*, edm::Service<TFileService>& );
    void branchesEvtSel_jet     ( TTree*, edm::Service<TFileService>& );
    void branchesEB             ( TTree*, edm::Service<TFileService>& );
    void branchesEE             ( TTree*, edm::Service<TFileService>& );
    void branchesHBHE           ( TTree*, edm::Service<TFileService>& );
    //void branchesECALatHCAL     ( TTree*, edm::Service<TFileService>& );
    void branchesECALstitched   ( TTree*, edm::Service<TFileService>& );
    void branchesHCALatEBEE     ( TTree*, edm::Service<TFileService>& );
    void branchesTracksAtEBEE   ( TTree*, edm::Service<TFileService>& );
    void branchesTracksAtECALstitched   ( TTree*, edm::Service<TFileService>& );
    void branchesPFCandsAtECALstitched   ( TTree*, edm::Service<TFileService>& );
    //void branchesTRKlayersAtEBEE( TTree*, edm::Service<TFileService>& );
    //void branchesTRKlayersAtECAL( TTree*, edm::Service<TFileService>& );
    //void branchesTRKvolumeAtEBEE( TTree*, edm::Service<TFileService>& );
    //void branchesTRKvolumeAtECAL( TTree*, edm::Service<TFileService>& );

    bool runEvtSel          ( const edm::Event&, const edm::EventSetup& );
    bool runEvtSel_jet      ( const edm::Event&, const edm::EventSetup& );
    void fillEB             ( const edm::Event&, const edm::EventSetup& );
    void fillEE             ( const edm::Event&, const edm::EventSetup& );
    void fillHBHE           ( const edm::Event&, const edm::EventSetup& );
    //void fillECALatHCAL     ( const edm::Event&, const edm::EventSetup& );
    void fillECALstitched   ( const edm::Event&, const edm::EventSetup& );
    void fillHCALatEBEE     ( const edm::Event&, const edm::EventSetup& );
    void fillTracksAtEBEE   ( const edm::Event&, const edm::EventSetup& );
    void fillTracksAtECALstitched   ( const edm::Event&, const edm::EventSetup& );
    void fillPFCandsAtECALstitched   ( const edm::Event&, const edm::EventSetup& );
    //void fillTRKlayersAtEBEE( const edm::Event&, const edm::EventSetup& );
    //void fillTRKlayersAtECAL( const edm::Event&, const edm::EventSetup& );
    //void fillTRKvolumeAtEBEE( const edm::Event&, const edm::EventSetup& );
    //void fillTRKvolumeAtECAL( const edm::Event&, const edm::EventSetup& );

    bool has_w2jet_z2invisible( const edm::Event&, const edm::EventSetup& );
    bool has_dijet( const edm::Event&, const edm::EventSetup& );

}; // class RecHitAnalyzer

//
// constants, enums and typedefs
//
static const int nEE = 2;
static const int EB_IPHI_MIN = 1;
static const int EB_IPHI_MAX = 360;
static const int EB_IETA_MIN = 1;
static const int EB_IETA_MAX = 85;
static const int EE_MIN_IX = 1;
static const int EE_MIN_IY = 1;
static const int EE_MAX_IX = 100;
static const int EE_MAX_IY = 100;
static const int EE_NC_PER_ZSIDE = EE_MAX_IX*EE_MAX_IY; // 100*100
static const int HBHE_IETA_MAX_FINE = 20;
static const int HBHE_IETA_MAX_HB = 16;
static const int HBHE_IETA_MIN_HB = 1;
static const int HBHE_IETA_MAX_HE = 29;
static const int HBHE_IETA_MAX_EB = HBHE_IETA_MAX_HB + 1; // 17
static const int HBHE_IPHI_NUM = 72;
static const int HBHE_IPHI_MIN = 1;
static const int HBHE_IPHI_MAX = 72;
static const int ECAL_IETA_MAX_EXT = 140;

// EE-(phi,eta) projection eta edges
// These are generated by requiring 5 fictional crystals
// to uniformly span each HCAL tower in eta (as in EB).
static const double eta_bins_EEm[5*(HBHE_IETA_MAX_HE-1-HBHE_IETA_MAX_EB)+1] =
                  {-3.    , -2.93  , -2.86  , -2.79  , -2.72  , -2.65  , -2.62  ,
                   -2.59  , -2.56  , -2.53  , -2.5   , -2.4644, -2.4288, -2.3932,
                   -2.3576, -2.322 , -2.292 , -2.262 , -2.232 , -2.202 , -2.172 ,
                   -2.1462, -2.1204, -2.0946, -2.0688, -2.043 , -2.0204, -1.9978,
                   -1.9752, -1.9526, -1.93  , -1.91  , -1.89  , -1.87  , -1.85  ,
                   -1.83  , -1.812 , -1.794 , -1.776 , -1.758 , -1.74  , -1.7226,
                   -1.7052, -1.6878, -1.6704, -1.653 , -1.6356, -1.6182, -1.6008,
                   -1.5834, -1.566 , -1.5486, -1.5312, -1.5138, -1.4964, -1.479 }; // 56
// EE+(phi,eta) projection eta edges
static const double eta_bins_EEp[5*(HBHE_IETA_MAX_HE-1-HBHE_IETA_MAX_EB)+1] =
                   {1.479 ,  1.4964,  1.5138,  1.5312,  1.5486,  1.566 ,  1.5834,
                    1.6008,  1.6182,  1.6356,  1.653 ,  1.6704,  1.6878,  1.7052,
                    1.7226,  1.74  ,  1.758 ,  1.776 ,  1.794 ,  1.812 ,  1.83  ,
                    1.85  ,  1.87  ,  1.89  ,  1.91  ,  1.93  ,  1.9526,  1.9752,
                    1.9978,  2.0204,  2.043 ,  2.0688,  2.0946,  2.1204,  2.1462,
                    2.172 ,  2.202 ,  2.232 ,  2.262 ,  2.292 ,  2.322 ,  2.3576,
                    2.3932,  2.4288,  2.4644,  2.5   ,  2.53  ,  2.56  ,  2.59  ,
                    2.62  ,  2.65  ,  2.72  ,  2.79  ,  2.86  ,  2.93  ,  3.    }; // 56
// MGG80, pt/m0 cut
static const int runTotal[3] = {14907, 22323, 20195}; //57425
// MGG80
//static const int runTotal[3] = {21918, 29913, 32805}; //84636
//static const int runTotal[3] = {6575, 8974, 9842}; //25391
//static const int runTotal[3] = {28493, 38887, 42647}; //84636+25391
//static const int runTotal[3] = {45363, 67946, 61752}; //175061
// MGG90
//static const int runTotal[3] = {16308, 24538, 22206}; //63052
//static const int runTotal[3] = {4892, 7361, 6662}; //18915
//static const int runTotal[3] = {21200, 31899, 28868}; //63052+18915
//static const int runTotal[3] = {35141, 47885, 52576}; //135602

//
// static data member definitions
//

//
// constructors and destructor
//
//RecHitAnalyzer::~RecHitAnalyzer()
//{
//
//  // do anything here that needs to be done at desctruction time
//  // (e.g. close files, deallocate resources etc.)
//
//}
#endif
