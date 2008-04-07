#ifndef XML2_H                                                                                                                                              
#define XML2_H  
/* buffered reading from file */


//void xmlWriteMap(QString filename);

/* Loads a char, returns char struct or NULL */
//void xmlReadMap(QString filename);

#include <QString>

class QProgressDialog;
class CRoomManager;
class CRoom;
class CRegion;

class StructureParser: public QXmlDefaultHandler
{
public:
  StructureParser(QProgressDialog *progress, unsigned int& currentMaximum, CRoomManager *parent);
  bool characters(const QString& ch);
    
  bool startElement( const QString&, const QString&, const QString& ,
		     const QXmlAttributes& );
  bool endElement( const QString&, const QString&, const QString& );

private:
  /* some flags */
  int flag;
  bool readingRegion;
  bool abortLoading;
  CRoomManager *parent;
  
  QProgressDialog *progress;
  unsigned int currentMaximum;

  char data[MAX_DATA_LEN];
  QString s;


  int i;
  CRoom *r;
  CRegion *region;
    
};

#endif
