TEMPLATE = subdirs

CONFIG += ordered c++11

SUBDIRS = thirdparty \
          client     

RESOURCES += \
    client/forms/resources.qrc
