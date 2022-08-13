## AWS CodeBuild Stack Setup

create or update the stack
```
aws cloudformation deploy --capabilities CAPABILITY_IAM --stack-name aws-lambda-cpp-ci --template-file codebuild.yml
```

(optional) trigger docker build and docker push of the build environment images.
A project to do this is pre-configured in the deployed stack.
```
aws cloudformation describe-stacks --stack-name aws-lambda-cpp-ci --query "Stacks[].Outputs[].OutputValue"
# run command output from above, will look like:
# aws codebuild start-build --project-name <value>
```
