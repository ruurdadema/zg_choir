#ifndef FridgeClientCanvas_h
#define FridgeClientCanvas_h

#include <QWidget>

#include "zg/messagetree/gateway/ITreeGatewaySubscriber.h"
#include "MagnetState.h"

namespace fridge {

/** Fridge-magnets drawing area. */
class FridgeClientCanvas : public QWidget, public ITreeGatewaySubscriber
{
Q_OBJECT

public:
   /** Constructor
     * @param connector the ITreeGateway we should register with and use for our database access
     */
   FridgeClientCanvas(ITreeGateway * connector);

   /** Destructor */
   virtual ~FridgeClientCanvas();

   virtual void paintEvent(QPaintEvent * e);
   virtual void mousePressEvent(QMouseEvent * e);
   virtual void mouseMoveEvent(QMouseEvent * e);
   virtual void mouseReleaseEvent(QMouseEvent * e);
   
   // ITreeGatewaySubscriber API
   virtual void TreeGatewayConnectionStateChanged();
   virtual void TreeNodeUpdated(const String & nodePath, const MessageRef & optPayloadMsg);

   /** Uploads a request to the server to clear all the magnets from the fridge */
   void ClearMagnets();

signals:
   void UpdateWindowStatus();

private:
   String GetMagnetAtPoint(const QPoint & pt) const;
   status_t UploadMagnetState(const String & optNodeID, const MagnetState * optMagnetState);
   String GetNextMagnetWord();
   void UpdateDraggedMagnetPosition(QPoint mousePos);

   Hashtable<String, MagnetState> _magnets;
   String _draggingID;  // if non-empty, we're in the middle of moving a magnet
   QPoint _dragDelta;   // mouse-click position minus upper-left position

   Queue<String> _magnetWords;
   uint32 _nextMagnetWordIndex;
};

}; // end namespace fridge

#endif
