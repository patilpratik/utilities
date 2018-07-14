#!/bin/bash

# This will build package. It will use Package.config file to get basic info of package

source ./Package.config

TRUE=1
FALSE=0

RELEASE=${FALSE}
BUILD_VERSION=""
ADD_GIT_TAG=${FALSE}


usage()
{
    echo "Build debian package. Make sure to have correct info in Package.config"
    echo ""
    echo "./build-package.sh"
    echo "-h --help"
    echo "--build=[release|debug]"
    echo "--tag=[tag]"
    echo ""
}

# validate arguments

while [ "$1" != "" ]; do
    PARAM=`echo $1 | awk -F= '{print $1}'`
    VALUE=`echo $1 | awk -F= '{print $2}'`
    case $PARAM in
        -h | --help)
            usage
            exit
            ;;
        -b | --build)
            if [ ${VALUE} = "release" ]; then
              RELEASE=${TRUE}
            elif [ ${VALUE} = "debug" ]; then
              RELESE=${FALSE}
            else
              echo "Incorrect build type"
              usage
              exit
            fi
            ;;
        -t | --tag)
            if [ -z ${VALUE} ]; then 
              GIT_TAG=${BASE_VERSION}
            else
              GIT_TAG=${VALUE}
            fi
              ADD_GIT_TAG=${TRUE}
              echo "$GIT_TAG"
            ;;
        *)
            echo "ERROR: unknown parameter \"$PARAM\""
            usage
            exit 1
            ;;
    esac
    shift
done

# Check for release build.
if [ ${RELEASE} -eq ${TRUE} ]; then
  echo "Building release"
  BUILD_VERSION="${BASE_VERSION}.${BUILD_NO}"
else
  echo "building debug builds"
  if [ ${ADD_GIT_TAG} -eq ${TRUE} ]; then
    # set tag on git
    echo "Adding tag in git"
    git tag -a v${GIT_TAG} -m 'Version ${GIT_TAG}'
    git push --tags
  fi
  
  # get build version from git.
  BUILD_VERSION=`git describe --long --tags --dirty --always`
fi

echo "Making Package directory for ${PKG_NAME}"
if [ -d ${PKG_NAME} ]; then
  echo "Removing existing directory ${PKG_NAME}"
  rm -rf ${PKG_NAME}
fi
mkdir ${PKG_NAME}

# Creating Package.
# as this one is a library, putting it into /usr/lib
mkdir -p ${PKG_NAME}/usr/lib/${PKG_NAME}/
 
echo "Copying Deliverables in ${PKG_NAME}"
cp -r Deliverable/artifacts/* ${PKG_NAME}/usr/lib/${PKG_NAME}/

# Copying installation scripts
cp -r Deliverable/DEBIAN ${PKG_NAME}

dpkg-deb --build ${PKG_NAME}

mv ${PKG_NAME}.deb ${PKG_NAME}_${BUILD_VERSION}.deb