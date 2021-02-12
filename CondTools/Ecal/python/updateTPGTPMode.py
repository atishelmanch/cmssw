import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

process = cms.Process("ProcessOne")

#-- Options that can be set on the command line 
options = VarParsing.VarParsing('analysis')                             
options.register ('inputTxtFile',
                'file.txt', # default value -- 0 = Run 2 
                VarParsing.VarParsing.multiplicity.singleton, 
                VarParsing.VarParsing.varType.string,          
                "inputFile")    
options.register ('TPModeTag',
                'EcalTPG_TPMode_Run2_default', # default value -- 0 = Run 2 
                VarParsing.VarParsing.multiplicity.singleton, 
                VarParsing.VarParsing.varType.string,          
                "TPModeTag")  
options.register ('outputDBFile',                                        
                'output.db', 
                VarParsing.VarParsing.multiplicity.singleton, 
                VarParsing.VarParsing.varType.string,          
                "outputDBFile")     
options.parseArguments()


process.MessageLogger = cms.Service("MessageLogger",
    cerr = cms.untracked.PSet(
        enable = cms.untracked.bool(False)
    ),
    cout = cms.untracked.PSet(
        enable = cms.untracked.bool(True),
        threshold = cms.untracked.string('DEBUG')
    ),
    debugModules = cms.untracked.vstring('*')
)

process.source = cms.Source("EmptyIOVSource",
    lastValue = cms.uint64(100000000000),
    timetype = cms.string('runnumber'),
    firstValue = cms.uint64(100000000000),
    interval = cms.uint64(1)
)

process.load("CondCore.CondDB.CondDB_cfi")

process.CondDB.connect = 'sqlite_file:%s' %(options.outputDBFile)

process.PoolDBOutputService = cms.Service("PoolDBOutputService",
  process.CondDB, 
  logconnect = cms.untracked.string('sqlite_file:log.db'),   
  toPut = cms.VPSet(
    cms.PSet(
      record = cms.string('EcalTPGTPModeRcd'),
      tag = cms.string(options.TPModeTag)
    )
  )
)

process.Test1 = cms.EDAnalyzer("ExTestEcalTPGTPModeAnalyzer",
  record = cms.string('EcalTPGTPModeRcd'),
  loggingOn= cms.untracked.bool(True),
  IsDestDbCheckedInQueryLog=cms.untracked.bool(True),
  SinceAppendMode=cms.bool(True),
  Source=cms.PSet(
    firstRun = cms.string('1'),
    lastRun = cms.string('10'),
    OnlineDBSID = cms.string(''),
    OnlineDBUser = cms.string(''),
    OnlineDBPassword = cms.string(''),
    LocationSource = cms.string(''),
    Location = cms.string(''),
    GenTag = cms.string(''),
    RunType = cms.string(''),
    fileType = cms.string('txt'),
    fileName = cms.string(options.inputTxtFile),
  )
)

process.p = cms.Path(process.Test1)
