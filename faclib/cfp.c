#include "cfp.h"

/** look up table for CFPs **/
static double cfp3_5[3][3] = { /** (5/2)3 **/
  {-4.714045207910E-01, 
   5.270462766947E-01, 
   7.071067811865E-01},

  {0.000000000000E+00, 
   -8.451542547285E-01, 
   5.345224838248E-01},

  {0.000000000000E+00, 
   4.629100498863E-01, 
   -8.864052604279E-01}
};

static double cfp3_7[6][4] = {  /** (7/2)3 **/
  {-5.000000000000E-01, 
   3.726779962500E-01, 
   5.000000000000E-01, 
   6.009252125773E-01},

  {0.000000000000E+00, 
   4.629100498863E-01, 
   -8.864052604279E-01, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   7.817359599706E-01, 
   2.461829819587E-01, 
   -5.729597091269E-01},

  {0.000000000000E+00, 
   3.212080372198E-01, 
   -8.058229640254E-01, 
   4.974683381631E-01},

  {0.000000000000E+00, 
   -5.270462766947E-01, 
   4.438126822993E-01, 
   7.247430753395E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   4.767312946228E-01, 
   8.790490729915E-01}
}; 

static double cfp3_9[10][5] = { /** (9/2)3 **/
  {-5.163977794943E-01, 
   2.886751345948E-01, 
   3.872983346207E-01, 
   4.654746681256E-01, 
   5.322906474224E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   -8.528028654224E-01, 
   5.222329678671E-01, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   -5.270462766947E-01, 
   4.438126822993E-01, 
   7.247430753395E-01, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   7.247430753395E-01, 
   3.739787960034E-01, 
   4.494665749755E-02, 
   -5.769463864755E-01},

  {0.000000000000E+00, 
   -1.811857688349E-01, 
   6.544628930059E-01, 
   -6.966731912120E-01, 
   2.312931116495E-01},

  {0.000000000000E+00, 
   4.143877070054E-01, 
   -6.295001161798E-01, 
   -3.363499860730E-01, 
   5.646955984255E-01},

  {0.000000000000E+00, 
   5.504818825632E-01, 
   -2.508726030021E-01, 
   -5.075192189226E-01, 
   -6.135608172438E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   3.645094413964E-01, 
   -7.977240352175E-01, 
   4.803844614153E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   5.397918963594E-01, 
   -4.156047072968E-01, 
   -7.320501594136E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   -4.830458915396E-01, 
   8.755950357709E-01}
};

static double cfp4_7[8][6] = { /** (7/2)4 **/
  {1.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00},

  {5.773502691896E-01, 
   2.535462764186E-01, 
   -5.244044240851E-01, 
   -2.781743201321E-01, 
   -5.000000000000E-01, 
   0.000000000000E+00},

  {5.773502691896E-01, 
   -3.618734322279E-01, 
   -1.230914909793E-01, 
   5.201564866103E-01, 
   3.138229572304E-01, 
   3.892494720808E-01},

  {5.773502691896E-01, 
   0.000000000000E+00, 
   2.383656473114E-01, 
   -2.671833572417E-01, 
   4.264014327112E-01, 
   -5.971962463406E-01},

  {0.000000000000E+00, 
   -5.606119105814E-01, 
   -1.581138830084E-01, 
   -7.548544196103E-01, 
   3.015113445778E-01, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   1.759328876372E-01, 
   6.582805886044E-01, 
   1.283881477533E-01, 
   6.454972243679E-01, 
   -3.202563076102E-01},

  {0.000000000000E+00, 
   3.872983346207E-01, 
   -3.337119062360E-01, 
   -5.539117094070E-01, 
   4.593897702811E-01, 
   4.698714938994E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   3.739787960034E-01, 
   4.834937784152E-01, 
   7.914376958255E-01}
};

static double cfp4_9[18][10] = { /** (9/2)4 **/
  {1.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00},

  {6.123724356958E-01, 
   0.000000000000E+00, 
   2.886751345948E-01, 
   4.583677673016E-01, 
   1.281176857976E-01, 
   3.209833376210E-01, 
   -4.605661864718E-01, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00},

  {6.123724356958E-01, 
   -2.842676218075E-01, 
   -1.811857688349E-01, 
   1.762952951160E-01, 
   -3.449322309936E-01, 
   -3.634420615313E-01, 
   1.564465546937E-01, 
   2.430062942643E-01, 
   -3.816905103453E-01, 
   0.000000000000E+00},

  {6.123724356958E-01, 
   1.448413648756E-01, 
   -2.461829819587E-01, 
   1.762952951160E-02, 
   3.055114045944E-01, 
   -1.615773069067E-01, 
   2.633387919531E-01, 
   -4.424976788687E-01, 
   2.445203668209E-01, 
   3.141941258489E-01},

  {6.123724356958E-01, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   -1.978909779673E-01, 
   -8.869685939836E-02, 
   2.372196000040E-01, 
   2.783986844542E-01, 
   2.330206912142E-01, 
   3.766366997783E-01, 
   -4.980353558597E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   1.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   5.563486402642E-01, 
   -1.868706368605E-01, 
   -2.659080117392E-01, 
   -6.662003030792E-01, 
   -3.755338080994E-01, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   3.872983346207E-01, 
   -2.598700974188E-01, 
   9.869275424397E-02, 
   6.477502756313E-01, 
   -3.447914135838E-01, 
   -9.160572248287E-02, 
   4.752621654141E-01, 
   0.000000000000E+00, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   -1.592555143177E-01, 
   1.687370337980E-01, 
   -5.502681446858E-01, 
   1.927397782733E-01, 
   -5.182837783900E-01, 
   4.006683379765E-01, 
   -4.207952582916E-01, 
   0.000000000000E+00, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   4.620893528835E-01, 
   -3.542542411790E-01, 
   -2.865756512070E-01, 
   -2.912739258160E-01, 
   5.054894886232E-02, 
   5.086213249304E-01, 
   3.464322623388E-01, 
   3.357753684224E-01, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   2.335496832485E-01, 
   3.567530340063E-01, 
   2.709733813986E-01, 
   4.580286124143E-01, 
   2.438043466400E-01, 
   4.996502273094E-01, 
   1.630135778806E-01, 
   -4.464310689241E-01, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   4.113766756037E-01, 
   2.253638847308E-01, 
   1.706971854997E-01, 
   3.192320631979E-01, 
   -7.822327734684E-01, 
   -2.039813511289E-01, 
   5.764553383343E-01, 
   3.156744361686E-01, 
   2.212488394344E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   3.800647506638E-01, 
   -5.987740463066E-01, 
   8.520269359026E-02, 
   1.173872680802E-01, 
   3.682868421967E-01, 
   4.103077963861E-02, 
   4.859555718072E-01, 
   -3.201712296361E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   3.322493196225E-01, 
   4.037783588022E-01, 
   -2.709733813986E-01, 
   4.807268595068E-01, 
   3.682868421967E-01, 
   1.756459548061E-01, 
   8.673306425112E-02, 
   4.980353558597E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   2.577470978179E-01, 
   4.081643146756E-01, 
   -3.512797117646E-01, 
   -5.613154352602E-01, 
   5.616415017345E-01, 
   1.142571466580E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   4.385290096535E-01, 
   2.853908964927E-01, 
   1.570970629244E-01, 
   -4.901020787860E-01, 
   -5.036630526336E-01, 
   -4.557283604224E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   3.226025390322E-01, 
   -5.695252183696E-01, 
   2.021555664953E-01, 
   4.021998332699E-01, 
   -6.074018838492E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   3.934447376823E-01, 
   -4.789694820193E-01, 
   -7.847225456709E-01}
};

static double cfp5_9[20][18] = { /** (9/2)5 **/
  {3.464101615138E-01, 
   -3.162277660168E-01, 
   -4.242640687119E-01, 
   -5.099019513593E-01, 
   -5.830951894845E-01, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00},
  
  {0.000000000000E+00, 
   0.000000000000E+00, 
   5.393598899706E-01, 
   -3.302891295379E-01, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   -3.240370349204E-01, 
   -1.510830465556E-01, 
   4.383764512876E-01, 
   -2.449489742783E-01, 
   4.690415759823E-01, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   3.333333333333E-01, 
   -2.806917861069E-01, 
   -4.583677673016E-01, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   -3.212080372198E-01, 
   -1.775250729197E-01, 
   -1.307031443576E-01, 
   2.744041552819E-01, 
   3.055050463304E-01, 
   -2.098023589051E-01, 
   -3.538210273630E-01, 
   3.322493196225E-01, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   4.583677673016E-01, 
   2.365249583956E-01, 
   2.842676218075E-02, 
   -3.648929338133E-01, 
   0.000000000000E+00, 
   9.343531843023E-02, 
   5.838742081211E-02, 
   3.691310932110E-01, 
   1.922407910386E-01, 
   2.009592381171E-01, 
   -1.376204706408E-01, 
   4.827470694315E-01, 
   3.496823162211E-01, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   1.145919418254E-01, 
   -4.139186771924E-01, 
   4.406148138016E-01, 
   -1.462826079840E-01, 
   -2.000000000000E-01, 
   1.189176780021E-01, 
   3.427572281988E-01, 
   -1.156438669640E-01, 
   1.747643554896E-01, 
   3.038218101251E-01, 
   -2.302015145264E-01, 
   -6.144053610947E-02, 
   -2.098950786844E-01, 
   -2.125437017999E-01, 
   3.823007273781E-01, 
   0.000000000000E+00, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   2.620817977023E-01, 
   -3.981308308938E-01, 
   -2.127264093913E-01, 
   3.571448551393E-01, 
   0.000000000000E+00, 
   2.719751348386E-01, 
   -1.665500757698E-01, 
   3.775900012021E-01, 
   -2.768679955007E-02, 
   1.476308632870E-01, 
   2.340584775202E-01, 
   -7.727375992650E-02, 
   3.399252222558E-01, 
   -3.072549338995E-01, 
   2.271204755350E-01, 
   -2.699086488667E-01, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   3.481553119114E-01, 
   -1.586657656044E-01, 
   -3.209833376210E-01, 
   -3.880499331049E-01, 
   0.000000000000E+00, 
   -1.419384378754E-01, 
   4.096732451994E-02, 
   2.031759303455E-01, 
   2.579180860863E-01, 
   -2.801098685544E-01, 
   -1.243163121016E-01, 
   2.244521950857E-01, 
   -2.411003331175E-01, 
   -2.448180904997E-01, 
   -1.157472932618E-01, 
   -4.411523371981E-01, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   2.305360126897E-01, 
   -5.045249791095E-01, 
   3.038218101251E-01, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   1.275783044887E-01, 
   1.996007167200E-01, 
   -1.643272505933E-01, 
   6.426845869172E-02, 
   -3.286301052180E-01, 
   -2.339114330830E-02, 
   1.075607411647E-01, 
   3.659328935626E-01, 
   -3.377795473744E-01, 
   -1.464756237711E-01, 
   -3.110453761209E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   3.413943709995E-01, 
   -2.628514962691E-01, 
   -4.629891730472E-01, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   1.501633097925E-01, 
   2.207214278632E-01, 
   1.696699112627E-01, 
   2.611932716088E-01, 
   -5.007535799303E-02, 
   3.452052529535E-01, 
   3.272733462066E-01, 
   2.747547926396E-01, 
   -3.570027736477E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   3.055050463304E-01, 
   -5.537749241945E-01, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   -1.075650869654E-01, 
   6.226337050660E-02, 
   2.600904819933E-01, 
   -6.352234031660E-02, 
   -2.678560818246E-01, 
   3.753223588664E-01, 
   5.290598323631E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   6.569783507542E-01, 
   -2.199532828587E-01, 
   -7.211102550928E-01, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   3.380617018914E-01, 
   7.784989441615E-02, 
   5.086894440271E-01, 
   4.365612200896E-01, 
   2.679456508228E-01, 
   -4.954084190119E-02, 
   -3.723858833378E-01, 
   -4.662430882947E-01, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   5.253323005242E-01, 
   -1.203687516079E+00, 
   -4.576542583313E-01, 
   2.416035184986E-01, 
   8.285761652006E-01, 
   1.083263497335E+00, 
   4.151137387766E-01, 
   -2.120260478221E-01, 
   -9.644013324700E-01, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   2.828427124746E-01, 
   2.522624895548E-01, 
   -2.423659603601E-01, 
   -2.453176875987E-01, 
   3.707311826292E-01, 
   -2.148344622118E-01, 
   -4.883311558831E-01, 
   -1.303350591682E-01, 
   1.484182334754E-01, 
   -4.508732785237E-01, 
   -2.703274367816E-01, 
   0.000000000000E+00, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   3.645094413964E-01, 
   7.440517815401E-02, 
   -2.572627301619E-01, 
   -2.207848672344E-01, 
   3.693375913852E-01, 
   -2.844144163547E-01, 
   5.050140351518E-01, 
   1.948384919928E-01, 
   2.292552471536E-01, 
   -2.541955637209E-01, 
   -3.452394580589E-01, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   2.141358117040E-01, 
   3.777000697079E-01, 
   -2.270532754512E-01, 
   3.941952143925E-01, 
   -1.519109050626E-01, 
   -2.267763173740E-01, 
   -1.470200688885E-01, 
   -1.606418744762E-01, 
   3.258930709701E-01, 
   5.649550496675E-01, 
   -2.392484461904E-01, 
   0.000000000000E+00},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   1.927442597079E-01, 
   1.138262323797E-02, 
   -3.100686151982E-01, 
   -3.480716106692E-01, 
   -1.636019247507E-01, 
   9.634664436434E-01, 
   5.019135388158E-01, 
   -4.493585171365E-01, 
   2.124517512433E-01, 
   1.798692335461E-01, 
   -3.208444739599E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   3.597235518407E-01, 
   1.036084593915E-01, 
   7.510676161988E-02, 
   2.624319405407E-02, 
   -4.862878750792E-01, 
   4.259892222771E-01, 
   1.848240217838E-01, 
   -3.037202484274E-02, 
   5.694583090428E-01, 
   2.760917653860E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   -2.974248450643E-01, 
   4.467513591020E-01, 
   8.733181225371E-02, 
   5.381760717116E+00, 
   1.863782232592E-01, 
   4.970501217477E-01, 
   -3.599442082970E-01, 
   4.224804167262E-01},

  {0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   0.000000000000E+00, 
   3.627381250550E-01, 
   -3.692744729380E-01, 
   -4.650238053994E-01, 
   7.181993293507E-01}  
};

/** look up the index of a given state. 
    j2:  2 times the angular momentum of the shell.
    q:   the occupation number.
    tj2: 2 times the coupled angular momentum.
    w:   other quantum numbers, including seneority.
**/

int GetIndex(int j2, int q, int tj2, int w) {
  switch (q) {
  case 1: /** one electron **/
    return 0;
  case 2: /** two electrons **/
    if (IsOdd(tj2/2)) return -1;
    return tj2 / 4;
  case 3: /** 3 electrons **/
    switch (j2) {
    case 5: /** 5/2 shell **/
      switch (tj2) {
      case 5: 
	return 0;
      case 3:
	return 1;
      case 9:
	return 2;
      default:
	return -1;
      }
    case 7: /** 7/2 shell **/
      switch (tj2) {
      case 7: 
	return 0;
      case 3:
	return 1;
      case 5:
	return 2;
      case 9:
	return 3;
      case 11:
	return 4;
      case 15:
	return 5;
      default: 
	return -1;
      }
      
    case 9: /** 9/2 shell **/
      switch (w) {
      case 1:
	if (tj2 == 9) return 0;
	return -1;
      case 3:
	switch (tj2) {
	case 3:
	  return 1;
	case 5:
	  return 2;
	case 7:
	  return 3;
	case 9:
	  return 4;
	case 11:
	  return 5;
	case 13:
	  return 6;
	case 15:
	  return 7;
	case 17:
	  return 8;
	case 21:
	  return 9;
	default: 
	  return -1;
	}
      }
    }

  case 4: /** 4 electrons **/
    switch (j2) {
    case 7: /** 7/2 shell **/
      switch (w) {
      case 0:
	if (tj2 == 0) return 0;
	return -1;
      case 2:
	switch (tj2) {
	case 4:
	  return 1;
	case 8:
	  return 2;
	case 12:
	  return 3;
	default: 
	  return -1;
	}
      case 4:
	switch (tj2) {
	case 4:
	  return 4;
	case 8:
	  return 5;
	case 10:
	  return 6;
	case 16:
	  return 7;
	default: 
	  return -1;
	}
      }
    case 9: /** 9/2 shell **/
      switch (w&0x0f) {
      case 0: 
	if (tj2 == 0) return 0;
	return -1;
      case 2:
	switch (tj2) {
	case 4:
	  return 1;
	case 8:
	  return 2;
	case 12:
	  return 3;
	case 16:
	  return 4;
	default: 
	  return -1;
	}
      case 4:
	switch (tj2) {
	case 0:
	  return 5;
	case 4:
	  return 6;
	case 6:
	  return 7;
	case 8:
	  if (w&0xf0) return 9;
	  else return 8;
	case 10:
	  return 10;
	case 12:
	  if (w&0xf0) return 12;
	  else return 11;
	case 14:
	  return 13;
	case 16:
	  return 14;
	case 18:
	  return 15;
	case 20:
	  return 16;
	case 24:
	  return 17;
	default: 
	  return -1;
	}
      }
    }

  case 5: /** 5 electrons, only 9/2 shell allowed **/
    if (j2 != 9) return -1;
    switch (w) {
    case 1:
      if (tj2 == 9) return 0;
      return -1;
    case 3:
      switch (tj2) {
      case 3:
	return 1;
      case 5:
	return 2;
      case 7:
	return 3;
      case 9:
	return 4;
      case 11:
	return 5;
      case 13:
	return 6;
      case 15:
	return 7;
      case 17:
	return 8;
      case 21:
	return 9;
      default: 
	return -1;
      }
    case 5:
      switch (tj2) {
      case 1:
	return 10;
      case 5:
	return 11;
      case 7:
	return 12;
      case 9:
	return 13;
      case 11:
	return 14;
      case 13:
	return 15;
      case 15:
	return 16;
      case 17:
	return 17;
      case 19:
	return 18;
      case 25:
	return 19;
      default: 
	return -1;
      }
    }
  default:
    return -1;
  }  
}


/** calculate the coeff. of fractional parentage **/
/** j2:  twice of shell angular momentum.
    q:   occupation number of the daughter state.
    dj:  twice of the daughter angular momentum.
    dw:  seneority of the daughter state.
    pj:  twice of the daughter parent momentum.
    pw:  seneority of the parent state.
    
    CFPs of upto 9/2 shell with any occupation # are available.
    for shells above 9/2, only 2 electrons are allowed.
**/

int CFP(double *coeff, int j2, int q, 
	int dj, int dw, int pj, int pw) {
  int i, j;
  int half_maxq, more_than_half = 0;

  if (IsEven(j2)) return -1;

  half_maxq = (j2 + 1) / 2;

  if (q > half_maxq) {
    if (q == half_maxq + 1) more_than_half = -1;
    else more_than_half = 1;
    q = half_maxq + half_maxq - q + 1;
    i = GetIndex(j2, q, pj, pw);
    j = GetIndex(j2, q - 1, dj, dw);
  } else {
    i = GetIndex(j2, q, dj, dw);
    j = GetIndex(j2, q - 1, pj, pw);
  }

  if (i < 0 || j < 0) return -1;

  switch (q) {
  case 2:
    *coeff = 1.0;
    break;
  case 3:
    switch (j2) {
    case 5:
      *coeff = cfp3_5[i][j];
      break;
    case 7:
      *coeff = cfp3_7[i][j];
      break;
    case 9:
      *coeff = cfp3_9[i][j];
      break;
    }
    break;
  case 4:
    switch (j2) {
    case 7:
      *coeff = cfp4_7[i][j];
      break;
    case 9:
      *coeff = cfp4_9[i][j];
      break;
    }
    break;
  case 5:
    *coeff = cfp5_9[i][j];
    break;
  }
   
  if (more_than_half == 1) {
    *coeff *= sqrt((q*(pj+1.0)) / ((2*half_maxq-q-1.0)*(dj+1.0)));
    if (IsOdd(((int)pj + (int)dj - (int)j2)/2))
      *coeff = - *coeff;
  } 
  if (more_than_half == -1) {
    *coeff *= sqrt((half_maxq*(pj+1.0)) / ((half_maxq+1.0)*(dj+1.0)));
    if (IsOdd(((int)(pw&0x0f) + (int)pj + (int)dj - (int)j2)/2)) 
      *coeff = - *coeff;
  }
  return 0;
}




