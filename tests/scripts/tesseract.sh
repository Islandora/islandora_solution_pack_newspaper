#!/bin/sh
apt-get --yes --force-yes update
apt-get --yes --force-yes install build-essential checkinstall
# Go for Ubuntu's packages first
FROMSOURCE=0
UBUNTUDIST="`lsb_release -sc`" # "precise" means from source

if [ "$UBUNTUDIST" != "precise" ]; then
  echo "Installing Tesseract OCR using Ubuntu Packages"
  apt-get --yes install tesseract-ocr tesseract-ocr-eng
fi
# Check if install worked or already there
$(command -v tesseract --version > /dev/null 2>&1)
if [ "$?" -eq "1" ]; then
  printf "\n"
  echo "Tesseract could not be installed via apt-get"
  printf "\n"
  echo "Will try from source now"
  FROMSOURCE=1
fi
if [ "$FROMSOURCE" -eq 1 ]; then
  echo "Installing Tesseract OCR from Source"
  mkdir ~/tesseract
  cd ~/tesseract
  wget http://www.leptonica.org/source/leptonica-1.69.tar.gz
  tar xf leptonica-1.69.tar.gz && rm -rf leptonica-1.69.tar.gz
  cd leptonica-1.69
  ./configure
  make && checkinstall --pkgname=libleptonica --pkgversion="1.69" --backup=no --deldoc=yes --fstrans=no --default
  cd ~/tesseract
  wget https://github.com/tesseract-ocr/tesseract/archive/3.02.02.tar.gz
  tar xf 3.02.02.tar.gz && rm -rf 3.02.02.tar.gz
  cd tesseract-3.02.02
  ./autogen.sh
  ./configure
  make && checkinstall --pkgname=tesseract-ocr --pkgversion="3.02.02" --backup=no --deldoc=yes --fstrans=no --default && ldconfig
  mkdir ~/tesseract/langs
  cd ~/tesseract/langs
  wget https://raw.githubusercontent.com/tesseract-ocr/tessdata/master/eng.traineddata
  echo "Deploying English trained language file"
  cp eng.traineddata /usr/local/share/tessdata/
  cd ~ && rm -rf ~/tesseract
fi
# If this fails, then we are out of luck
printf "\n"
echo "tessceract output:"
tesseract --version && tesseract --list-langs