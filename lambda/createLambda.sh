#!/bin/sh

VV=$(git --git-dir ../.git --work-tree $$PWD describe --always --tags | tr "-" "_" | tr "." "-")
NAME=$1 #-$VV
echo $NAME

zip -r $NAME index.js ../bin ../expressjs/extraCash.sh ../expressjs/fullAnalysis.sh ../expressjs/evo.sh ../cache_categories.json
aws s3 cp $NAME.zip s3://cache.ai
rm -f $NAME.zip

aws lambda create-function --region us-west-2 \
    --function-name $NAME \
    --handler index.handler --runtime nodejs \
    --timeout 300 --memory-size 128 \
    --role arn:aws:iam::931303121402:role/lambda_basic_execution \
    --code S3Bucket=cache.ai,S3Key=$NAME.zip
