echo "building core"
cd docker
if [ "$1" == "en" ]
then
  echo "build en docker:"
  docker build -f Dockerfile.en -t cyborg/core:0.0 . --no-cache
else
  echo "build cn docker:"
  docker build -f Dockerfile.cn -t cyborg/core:0.0 . --no-cache
fi
docker tag cyborg/core:0.0 cyborg/core:latest
