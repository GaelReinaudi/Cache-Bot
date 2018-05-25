#!/bin/sh

VV=$(git --git-dir ../.git --work-tree $$PWD describe --always --tags | tr "-" "_" | tr "." "-")
NAME=$1-$VV
echo $NAME

#rm -f $NAME.zip
zip -r $NAME index.js ../bin ../expressjs/extraCash.sh ../expressjs/fullAnalysis.sh ../expressjs/evo.sh ../cache_categories.json
aws s3 cp $NAME.zip s3://cache.ai
rm -f $NAME.zip

aws lambda update-function-code \
    --region us-west-2 \
    --function-name cache \
    --s3-bucket=cache.ai --s3-key $NAME.zip \
    --publish

aws lambda update-alias --function-name cache \
    --name evo-dev \
    --function-version \$LATEST \
    --description $NAME

aws lambda update-alias --function-name cache \
    --name extra-dev \
    --function-version \$LATEST \
    --description $NAME

#aws lambda update-function-code --region us-west-2 --function-name extra --s3-bucket=cache.ai --s3-key $NAME.zip
#aws lambda update-function-code --region us-west-2 --function-name evo --s3-bucket=cache.ai --s3-key $NAME.zip
