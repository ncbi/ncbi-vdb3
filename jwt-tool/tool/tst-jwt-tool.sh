#! /bin/bash

TOOL=$PWD/bin/jwt-tool-dbg
[ ! -e $TOOL ] && { echo "Executable '$TOOL' doesnt exist."; exit 1; }

OUTDIR=$PWD/tool/input

#################################################################################################
# tool functionality tests
#################################################################################################
# import-pem

PWD=testjwttool
PRIVKEY=$OUTDIR/extPrivPem.jwk
PUBKEY=$OUTDIR/extPubPem.jwk

(! $TOOL import-pem "" 2> /dev/null &&
    echo "import-pem test - Missing pem file: Pass") ||
    { echo "import-pem test - Missing pem file: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/test_priv.pem 2> /dev/null &&
    echo "import-pem test - Missing pem file password: Pass") ||
    { echo "import-pem test - Missing pem file password: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/test_priv.pem --pwd 2> /dev/null &&
    echo "import-pem test - Missing pem file password2: Pass") ||
    { echo "import-pem test - Missing pem file password2: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/test_priv.pem --pwd "" 2> /dev/null &&
    echo "import-pem test - Missing pem file password3: Pass") ||
    { echo "import-pem test - Missing pem file password3: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/test_priv.pem --pwd incorrectpassword 2> /dev/null &&
    echo "import-pem test - Incorrect password: Pass") ||
    { echo "import-pem test - Incorrect password: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/test_priv.pem --pwd $PWD --kids 2> /dev/null &&
    echo "import-pem test - Missing kid(s): Pass") ||
    { echo "import-pem test - Missing kid(s): Failed"; exit 1; }

if [ 1 -eq 0 ]
then
(! $TOOL import-pem $OUTDIR/test_priv.pem --pwd $PWD --jws-policy 2> /dev/null &&
    echo "import-pem test - Missing jws policy options: Pass") ||
    { echo "import-pem test - Missing jws policy options: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/test_priv.pem --pwd $PWD --jws-policy blark 2> /dev/null &&
    echo "import-pem test - Invalid jws policy options: Pass") ||
    { echo "import-pem test - Invalid jws policy options: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/test_priv.pem --pwd $PWD --jws-policy blark=flark 2> /dev/null &&
    echo "import-pem test - Invalid jws policy options2: Pass") ||
    { echo "import-pem test - Invalid jws policy options2: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/test_priv.pem --pwd $PWD --jwt-policy 2> /dev/null &&
    echo "import-pem test - Missing jwt policy options: Pass") ||
    { echo "import-pem test - Missing jwt policy options: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/test_priv.pem --pwd $PWD --jwt-policy blark  2> /dev/null &&
    echo "import-pem test - Invalid jwt policy options: Pass") ||
    { echo "import-pem test - Invalid jwt policy options: Failed"; exit 1; }
(! $TOOL import-pem $OUTDIR/test_priv.pem --pwd $PWD --jwt-policy blark=flark 2> /dev/null  &&
    echo "import-pem test - Invalid jwt policy options2: Pass") ||
    { echo "import-pem test - Invalid jwt policy options2: Failed"; exit 1; }
fi
   
# import pem to hard-coded file path
$TOOL import-pem $OUTDIR/test_priv.pem --pwd $PWD 2> /dev/null
[ ! -e $OUTDIR/extPemPrivKey.jwk ] && { echo "Failed to create file: OUTDIR/extPemPrivKey.jwk"; exit 1; }
[ ! -e $OUTDIR/extPemPubKey.jwk ] && { echo "Failed to create file: OUTDIR/extPemPubKey.jwkY"; exit 1; }

#import pem to user specified file path
$TOOL import-pem $OUTDIR/test_priv.pem --pwd $PWD --priv-key $PRIVKEY --pub-key $PUBKEY 2> /dev/null
[ ! -e $PRIVKEY ] && { echo "Failed to create file: $PRIVKEY"; exit 1; }
[ ! -e $PUBKEY ] && { echo "Failed to create file: $PUBKEY"; exit 1; }

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

(! $TOOL sign $PAYLOAD --priv-key tool/input/extPrivPem.jwk --jws-policy 2> /dev/null &&
    echo "sign test - Missing jws policy options: Pass") ||
    { echo "sign test - Missing jws policy options: Failed"; exit 1; }
(! $TOOL sign $PAYLOAD --priv-key tool/input/extPrivPem.jwk --jws-policy blark 2> /dev/null &&
    echo "sign test - Invalid jws policy options: Pass") ||
    { echo "sign test - Invalid jws policy options: Failed"; exit 1; }
(! $TOOL sign $PAYLOAD --priv-key tool/input/extPrivPem.jwk --jws-policy blark=flark 2> /dev/null &&
    echo "sign test - Invalid jws policy options2: Pass") ||
    { echo "sign test - Invalid jws policy options2: Failed"; exit 1; }
(! $TOOL sign $PAYLOAD --priv-key tool/input/extPrivPem.jwk --jwt-policy 2> /dev/null &&
    echo "sign test - Missing jwt policy options: Pass") ||
    { echo "sign test - Missing jwt policy options: Failed"; exit 1; }
(! $TOOL sign $PAYLOAD --priv-key tool/input/extPrivPem.jwk --jwt-policy blark  2> /dev/null &&
    echo "sign test - Invalid jwt policy options: Pass") ||
    { echo "sign test - Invalid jwt policy options: Failed"; exit 1; }
(! $TOOL sign $PAYLOAD --priv-key tool/input/extPrivPem.jwk --jwt-policy blark=flark 2> /dev/null  &&
    echo "sign test - Invalid jwt policy options2: Pass") ||
    { echo "sign test - Invalid jwt policy options2: Failed"; exit 1; }

($TOOL sign $PAYLOAD --priv-key tool/input/extPrivPem.jwk 2> /dev/null &&
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



