#!/bin/sh

zip -r  z index.js ../bin ../expressjs/extraCash.sh ../expressjs/fullAnalysis.sh ../expressjs/evo.sh ../cache_categories.json

aws s3 cp z.zip s3://cache.ai

