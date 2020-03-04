# only need standard ubuntu image to install deb package and run hpcc
sudo docker run -ti -p 8010:8010 -v /home/mayx/git/run:/home/mayx/git/run --rm ubuntu:18.04 /bin/bas
