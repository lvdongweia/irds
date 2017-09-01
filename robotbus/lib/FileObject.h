#ifndef FILEOBJECT_H_
#define FILEOBJECT_H_

#include "DataObject.h"

namespace RobotNetwork
{
    class DLLEXPORT FileObject : public DataObject
    {
		private:
			char* name;
			FILE* fr;
			int mode;
			int objStatus;
			int fileSize;
			int position;
		private:
           void init(char* name_, int mode, int maxSize);
           int open(int mode);
		public:
           virtual ~FileObject();
           FileObject();
           FileObject(char* name);
           FileObject(char* name, int mode);
           FileObject(char* name, int mode, int maxSize);
           char* getName();
           int getMode();
           void Clear();
           int getType();
           int getSize();
           void setSize(int size);
           int getPosition();
           bool isEnd();
           void Reset();
			int read(DataBuffer * buffer);
			int write(DataBuffer * buffer);
    };
}

#endif


