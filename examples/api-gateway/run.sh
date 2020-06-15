rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=~/install
make
make aws-lambda-package-api
aws lambda update-function-code --function-name api --zip-file fileb://api.zip
