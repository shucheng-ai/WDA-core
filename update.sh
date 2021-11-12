echo "updating core"
git submodule init
git submodule update
cd docker
docker-compose up
