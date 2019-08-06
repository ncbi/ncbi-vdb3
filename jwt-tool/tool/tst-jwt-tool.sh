#! /bin/bash

TOOL=$PWD/bin/jwt-tool-dbg
[ ! -e $TOOL ] && { echo "Executable '$TOOL' doesnt exist."; exit 1; }

OUTDIR=$PWD/tool/input

#################################################################################################
# tool functionality tests

#################################################################################################
# import-pem

PWD=testjwttool

(! $TOOL import-pem "" 2> /dev/null &&
    echo "import-pem test - Missing pem file: Pass") ||
    { echo "import-pem test - Missing pem file: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/rs_private.pem 2> /dev/null &&
    echo "import-pem test - Missing pem file password: Pass") ||
    { echo "import-pem test - Missing pem file password: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/rs_private.pem --pwd 2> /dev/null &&
    echo "import-pem test - Missing pem file password2: Pass") ||
    { echo "import-pem test - Missing pem file password2: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/rs_private.pem --pwd "" 2> /dev/null &&
    echo "import-pem test - Missing pem file password3: Pass") ||
    { echo "import-pem test - Missing pem file password3: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/rs_private.pem --pwd incorrectpassword 2> /dev/null &&
    echo "import-pem test - Incorrect password: Pass") ||
    { echo "import-pem test - Incorrect password: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/rs_private.pem --pwd $PWD --kids 2> /dev/null &&
    echo "import-pem test - Missing kid(s): Pass") ||
    { echo "import-pem test - Missing kid(s): Failed"; exit 1; }

if [ 1 -eq 0 ]
then
(! $TOOL import-pem $OUTDIR/rs_private.pem --pwd $PWD --jws-policy 2> /dev/null &&
    echo "import-pem test - Missing jws policy options: Pass") ||
    { echo "import-pem test - Missing jws policy options: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/rs_private.pem --pwd $PWD --jws-policy blark 2> /dev/null &&
    echo "import-pem test - Invalid jws policy options: Pass") ||
    { echo "import-pem test - Invalid jws policy options: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/rs_private.pem --pwd $PWD --jws-policy blark=flark 2> /dev/null &&
    echo "import-pem test - Invalid jws policy options2: Pass") ||
    { echo "import-pem test - Invalid jws policy options2: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/rs_private.pem --pwd $PWD --jwt-policy 2> /dev/null &&
    echo "import-pem test - Missing jwt policy options: Pass") ||
    { echo "import-pem test - Missing jwt policy options: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/rs_private.pem --pwd $PWD --jwt-policy blark  2> /dev/null &&
    echo "import-pem test - Invalid jwt policy options: Pass") ||
    { echo "import-pem test - Invalid jwt policy options: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/rs_private.pem --pwd $PWD --jwt-policy blark=flark 2> /dev/null  &&
    echo "import-pem test - Invalid jwt policy options2: Pass") ||
    { echo "import-pem test - Invalid jwt policy options2: Failed"; exit 1; }
fi

rm $OUTDIR/importRSAPrivKey.jwk $OUTDIR/importRSAPubKey.jwk

$TOOL import-pem $OUTDIR/rs_private.pem --pwd $PWD 2> /dev/null
( [ -e $OUTDIR/importRSAPrivKey.jwk ] && echo "import-pem test - Create default priv file: Pass" ) ||
    { echo "import-pem test - Create default priv file: Failed"; exit 1; }
( [ -e $OUTDIR/importRSAPubKey.jwk ] && echo "import-pem test - Create default pub file: Pass" ) ||
    { echo "import-pem test - Create default pub file: Failed"; exit 1; }

#import pem to user specified file path
#$TOOL import-pem $OUTDIR/ec_private.pem --pwd $PWD --priv-key $PRIVKEY --pub-key $PUBKEY
#( [ -e $PRIVKEY ] && echo "Create user defined file: Pass") ||
#    { echo "Create user defined file: Failed"; exit 1; }

echo ------------------------------------------------------------------

#################################################################################################
# gen-key


(! $TOOL gen-key 2> /dev/null &&
    echo "gen-key test - Missing options: Pass") ||
    { echo "gen-key test - Missing options: Failed"; exit 1; }
(! $TOOL gen-key --type "" --use 2> /dev/null &&
    echo "gen-key test - Missing 'use' argument: Pass") ||
    { echo "gen-key test - Missing 'use' argument: Failed"; exit 1; }
(! $TOOL gen-key --type "" --use "" 2> /dev/null &&
    echo "gen-key test - Missing 'alg' argument: Pass") ||
    { echo "gen-key test - Missing 'alg' argument: Failed"; exit 1; }
(! $TOOL gen-key --type "" --use "" --alg "" 2> /dev/null &&
    echo "gen-key test - Invalid 'key' type: Pass") ||
    { echo "gen-key test - invalid 'key' type: Failed"; exit 1; }
(! $TOOL gen-key --type RSA --use "" --alg RS256 2> /dev/null &&
    echo "gen-key test - Invalid 'use' type: Pass") ||
    { echo "gen-key test - invalid 'use' type: Failed"; exit 1; }
(! $TOOL gen-key --type RSA --use sig --alg "" 2> /dev/null &&
    echo "gen-key test - Invalid 'alg' type: Pass") ||
    { echo "gen-key test - invalid 'alg' type: Failed"; exit 1; }
(! $TOOL gen-key --type EC --use sig --alg "" 2> /dev/null &&
    echo "gen-key test - Missing 'curve' argument: Pass") ||
    { echo "gen-key test - Missing 'curve' argument: Failed"; exit 1; }
(! $TOOL gen-key --type EC --curve "" --use sig --alg ES256 2> /dev/null &&
    echo "gen-key test - Invalid 'curve' type: Pass") ||
    { echo "gen-key test - invalid 'curve' type: Failed"; exit 1; }

rm $OUTDIR/generateRSAPrivKey.jwk $OUTDIR/generateRSAPubKey.jwk

$TOOL gen-key --type RSA --use sig --alg RS256 2> /dev/null
( [ -e $OUTDIR/generateRSAPrivKey.jwk ] && echo "gen-key test - Create default RSA priv file: Pass" ) ||
    { echo "gen-key test - Create default RSA priv file: Failed"; exit 1; }
( [ -e $OUTDIR/generateRSAPubKey.jwk ] && echo "gen-key test - Create default RSA pub file: Pass" ) ||
    { echo "gen-key test - Create default RSA pub file: Failed"; exit 1; }

rm $OUTDIR/generateECPrivKey.jwk $OUTDIR/generateECPubKey.jwk

$TOOL gen-key --type EC --curve secp256r1 --use sig --alg ES256 2> /dev/null
( [ -e $OUTDIR/generateECPrivKey.jwk ] && echo "gen-key test - Create default EC priv file: Pass" ) ||
    { echo "gen-key test - Create default EC priv file: Failed"; exit 1; }
( [ -e $OUTDIR/generateECPubKey.jwk ] && echo "gen-key test - Create default EC pub file: Pass" ) ||
    { echo "gen-key test - Create default EC pub file: Failed"; exit 1; }

exit 1;

echo ------------------------------------------------------------------

#################################################################################################
# sign: JSON text

PAYLOAD='{"iss":"https://dbgap.nlm.nih.gov/aa","sub":"00000","vcard":{"email":"user@nih.gov","fname":"My","lname":"Name","orgs":["NIH"],"roles":["have_approved_access","downloader"]}}'

(! $TOOL sign "" 2> /dev/null &&
    echo "sign test - Missing Payload: Pass") ||
    { echo "sign test - Missing Payload: Failed"; exit 1; }
(! $TOOL sign $PAYLOAD 2> /dev/null &&
    echo "sign test - Missing Signing Key: Pass") ||
    { echo "sign test - Missing Signing Key: Failed"; exit 1; }
(! $TOOL sign $PAYLOAD --priv-key 2> /dev/null &&
    echo "sign test - Missing Signing Key2: Pass") ||
    { echo "sign test - Missing Signing Key2: Failed"; exit 1; }
(! $TOOL sign $PAYLOAD --priv-key tool/input/extEmptyPrivPem.jwk 2> /dev/null &&
    echo "sign test - Empty Key: Pass") ||
    { echo "sign test - Empty Key: Failed"; exit 1; }

(! $TOOL sign $PAYLOAD --priv-key tool/input/extPemPrivKey.jwk --jws-policy 2> /dev/null &&
    echo "sign test - Missing jws policy options: Pass") ||
    { echo "sign test - Missing jws policy options: Failed"; exit 1; }
(! $TOOL sign $PAYLOAD --priv-key tool/input/extPemPrivKey.jwk --jws-policy blark 2> /dev/null &&
    echo "sign test - Invalid jws policy options: Pass") ||
    { echo "sign test - Invalid jws policy options: Failed"; exit 1; }
(! $TOOL sign $PAYLOAD --priv-key tool/input/extPemPrivKey.jwk --jws-policy blark=flark 2> /dev/null &&
    echo "sign test - Invalid jws policy options2: Pass") ||
    { echo "sign test - Invalid jws policy options2: Failed"; exit 1; }
(! $TOOL sign $PAYLOAD --priv-key tool/input/extPemPrivKey.jwk --jwt-policy 2> /dev/null &&
    echo "sign test - Missing jwt policy options: Pass") ||
    { echo "sign test - Missing jwt policy options: Failed"; exit 1; }
(! $TOOL sign $PAYLOAD --priv-key tool/input/extPemPrivKey.jwk --jwt-policy blark  2> /dev/null &&
    echo "sign test - Invalid jwt policy options: Pass") ||
    { echo "sign test - Invalid jwt policy options: Failed"; exit 1; }
(! $TOOL sign $PAYLOAD --priv-key tool/input/extPemPrivKey.jwk --jwt-policy blark=flark 2> /dev/null  &&
    echo "sign test - Invalid jwt policy options2: Pass") ||
    { echo "sign test - Invalid jwt policy options2: Failed"; exit 1; }

($TOOL sign $PAYLOAD --priv-key tool/input/extPemPrivKey.jwk 2> /dev/null &&
    echo "sign test - Succeed: Pass") ||
    { echo "sign test - Succeed: Failed"; exit 1; }

echo ------------------------------------------------------------------

#################################################################################################
# decode: JWT

JWT=$($TOOL sign $PAYLOAD --priv-key tool/input/extPrivPem.jwk 2> /dev/null)

(! $TOOL decode "" 2> /dev/null &&
    echo "decode test - Missing Payload: Pass") ||
    { echo "decode test - Missing Payload: Failed"; exit 1; }
(! $TOOL decode $JWT 2> /dev/null &&
    echo "decode test - Missing public decoding key: Pass") ||
    { echo "decode test - Missing public decoding key: Failed"; exit 1; }
(! $TOOL decode $JWT --pub-key 2> /dev/null &&
    echo "decode test - Missing public decoding key2: Pass") ||
    { echo "decode test - Missing public decoding key2: Failed"; exit 1; }
(! $TOOL decode $JWT --pub-key tool/input/extEmptyPubPem.jwk 2> /dev/null &&
    echo "decode test - Missing public decoding key3: Pass") ||
    { echo "decode test - Missing public decoding key3: Failed"; exit 1; }

(! $TOOL decode $PAYLOAD --pub-key tool/input/extPubPem.jwk --jws-policy 2> /dev/null &&
    echo "decode test - Missing jws policy options: Pass") ||
    { echo "decode test - Missing jws policy options: Failed"; exit 1; }
(! $TOOL decode $PAYLOAD --pub-key tool/input/extPubPem.jwk --jws-policy blark 2> /dev/null &&
    echo "decode test - Invalid jws policy options: Pass") ||
    { echo "decode test - Invalid jws policy options: Failed"; exit 1; }
(! $TOOL decode $PAYLOAD --pub-key tool/input/extPubPem.jwk --jws-policy blark=flark 2> /dev/null &&
    echo "decode test - Invalid jws policy options2: Pass") ||
    { echo "decode test - Invalid jws policy options2: Failed"; exit 1; }
(! $TOOL decode $PAYLOAD --pub-key tool/input/extPubPem.jwk --jwt-policy 2> /dev/null &&
    echo "decode test - Missing jwt policy options: Pass") ||
    { echo "decode test - Missing jwt policy options: Failed"; exit 1; }
(! $TOOL decode $PAYLOAD --pub-key tool/input/extPubPem.jwk --jwt-policy blark  2> /dev/null &&
    echo "decode test - Invalid jwt policy options: Pass") ||
    { echo "decode test - Invalid jwt policy options: Failed"; exit 1; }
(! $TOOL decode $PAYLOAD --pub-key tool/input/extPubPem.jwk --jwt-policy blark=flark 2> /dev/null  &&
    echo "decode test - Invalid jwt policy options2: Pass") ||
    { echo "decode test - Invalid jwt policy options2: Failed"; exit 1; }

($TOOL decode $JWT --pub-key tool/input/extPubPem.jwk 2> /dev/null &&
    echo "decode test - Succeed: Pass") ||
    { echo "decode test - Succeed: Failed"; exit 1; }

echo ------------------------------------------------------------------

#################################################################################################
# examine: JWT

(! $TOOL examine "" 2> /dev/null &&
    echo "examine test - Missing Payload: Pass") ||
    { echo "examine test - Missing Payload: Failed"; exit 1; }
(! $TOOL examine $JWT 2> /dev/null &&
    echo "examine test - Missing public decoding key: Pass") ||
    { echo "examine test - Missing public decoding key: Failed"; exit 1; }
(! $TOOL examine $JWT --pub-key 2> /dev/null &&
    echo "examine test - Missing public decoding key2: Pass") ||
    { echo "examine test - Missing public decoding key2: Failed"; exit 1; }
(! $TOOL examine $JWT --pub-key tool/input/extEmptyPubPem.jwk 2> /dev/null &&
    echo "examine test - Missing public decoding key3: Pass") ||
    { echo "examine test - Missing public decoding key3: Failed"; exit 1; }

(! $TOOL examine $PAYLOAD --pub-key tool/input/extPubPem.jwk --jws-policy 2> /dev/null &&
    echo "examine test - Missing jws policy options: Pass") ||
    { echo "examine test - Missing jws policy options: Failed"; exit 1; }
(! $TOOL examine $PAYLOAD --pub-key tool/input/extPubPem.jwk --jws-policy blark 2> /dev/null &&
    echo "examine test - Invalid jws policy options: Pass") ||
    { echo "examine test - Invalid jws policy options: Failed"; exit 1; }
(! $TOOL examine $PAYLOAD --pub-key tool/input/extPubPem.jwk --jws-policy blark=flark 2> /dev/null &&
    echo "examine test - Invalid jws policy options2: Pass") ||
    { echo "examine test - Invalid jws policy options2: Failed"; exit 1; }
(! $TOOL examine $PAYLOAD --pub-key tool/input/extPubPem.jwk --jwt-policy 2> /dev/null &&
    echo "examine test - Missing jwt policy options: Pass") ||
    { echo "examine test - Missing jwt policy options: Failed"; exit 1; }
(! $TOOL examine $PAYLOAD --pub-key tool/input/extPubPem.jwk --jwt-policy blark  2> /dev/null &&
    echo "examine test - Invalid jwt policy options: Pass") ||
    { echo "examine test - Invalid jwt policy options: Failed"; exit 1; }
(! $TOOL examine $PAYLOAD --pub-key tool/input/extPubPem.jwk --jwt-policy blark=flark 2> /dev/null  &&
    echo "examine test - Invalid jwt policy options2: Pass") ||
    { echo "examine test - Invalid jwt policy options2: Failed"; exit 1; }

($TOOL examine $JWT --pub-key tool/input/extPubPem.jwk 2> /dev/null &&
    echo "examine test - Succeed: Pass") ||
    { echo "examine test - Succeed: Failed"; exit 1; }



