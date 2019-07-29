#!/usr/bin/env bash
PDFium_BRANCH=chromium/3710

set -ex

OS="$(uname | tr '[:upper:]' '[:lower:]')"

BASEDIR="$PWD"
if [[ $# -ne 0 ]] ; then
    if [ -d "$1" ]; then
        BASEDIR="$1"
    else
        echo "must provide a valid folder"
        exit 1
    fi
fi

echo "using BASEDIR=$BASEDIR"

CURDIR="$PWD"
PDFIUMDIR="$BASEDIR/pdfium"
DepotTools_URL='https://chromium.googlesource.com/chromium/tools/depot_tools.git'
DepotTools_DIR="$PDFIUMDIR/depot_tools"
PDFium_URL='https://pdfium.googlesource.com/pdfium.git'
PDFium_BUILD_DIR="$PDFIUMDIR/out"
PDFium_ARGS="$PWD/args_$OS.gn"
outputDIR="$BASEDIR/pdfium-mega-1.0"

# Prepare directories
mkdir -p "$PDFium_BUILD_DIR"

# Download depot_tools if not exists in this location or update utherwise
if [ ! -d "$DepotTools_DIR" ]; then
  git clone "$DepotTools_URL" "$DepotTools_DIR"
else 
  cd "$DepotTools_DIR"
  git pull
  cd ..
fi
export PATH="$DepotTools_DIR:$PATH"

# Clone
cd "$BASEDIR"
gclient config --unmanaged "$PDFium_URL"
gclient sync

# Checkout
cd "$PDFIUMDIR"
git checkout "${PDFium_BRANCH:-master}"
gclient sync

# Configure
cp "$PDFium_ARGS" "$PDFium_BUILD_DIR/args.gn"

# Generate Ninja files
cd "$PDFIUMDIR"
gn gen "$PDFium_BUILD_DIR"

if [ "$OS" == "linux" ]; then
  cd "$PDFIUMDIR"
  echo "saving min pdfium stuff required for compilation"
  rsync -a --files-from="$CURDIR/filespdfium.txt" "$PDFIUMDIR" "$outputDIR"

  cd "$BASEDIR"
  tar -zcvf pdfium.tar.gz "pdfium-mega-1.0"

elif [ "$OS" == "darwin" ]; then
  # Build
  echo "Building PDFium"
  ninja -C "$PDFium_BUILD_DIR" pdfium
  ls -l "$PDFium_BUILD_DIR"

  # Copy required artifacts
  cp -R "$PDFIUMDIR/public" "$outputDIR"
  rm -f "$outputDIR/DEPS"
  rm -f "$outputDIR/README"
  rm -f "$outputDIR/PRESUBMIT.py"
  mv "$PDFIUMDIR/out/obj/libpdfium.a" "$outputDIR"

  cd "$BASEDIR"
  tar cvf "pdfium-$OS.tgz" "pdfium-mega-1.0"

  # Cleaning
  rm -rf "$PDFIUMDIR"
  rm -rf "$outputDIR"
fi


