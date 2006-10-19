/******************************************************************************\
* EventHandler.cpp                                                             *
* An event handler providing GUI-like events                                   *
*                                                                              *
*                                                                              *
* Copyright (C) 2006 by Leandro Motta Barros.                                  *
*                                                                              *
* This program is distributed under the OpenSceneGraph Public License. You     *
* should have received a copy of it with the source distribution, in a file    *
* named 'COPYING.txt'.                                                         *
\******************************************************************************/

#include "OSGUIsh/EventHandler.hpp"
#include <boost/lexical_cast.hpp>


namespace OSGUIsh
{

   // - EventHandler::EventHandler ---------------------------------------------
   EventHandler::EventHandler(
      osgProducer::Viewer& viewer,
      const FocusPolicyFactory& kbdPolicyFactory,
      const FocusPolicyFactory& wheelPolicyFactory)
      : viewer_(viewer), ignoreBackFaces_(false),
        kbdFocusPolicy_(kbdPolicyFactory.create (kbdFocus_)),
        wheelFocusPolicy_(wheelPolicyFactory.create (wheelFocus_))
   {
      addNode (NodePtr());

      for (int i = 0; i < MOUSE_BUTTON_COUNT; ++i)
      {
         nodeThatGotMouseDown_[i] = NodePtr();
         nodeThatGotClick_[i] = NodePtr();
         timeOfLastClick_[i] = -1.0;
      }

      // By default, use the viewer's scene root as the only root node when
      // picking
      pickingRoots_.push_back (viewer_.getSceneData());
   }



   // - EventHandler::handle ---------------------------------------------------
   bool EventHandler::handle (const osgGA::GUIEventAdapter& ea,
                              osgGA::GUIActionAdapter&)
   {
      switch (ea.getEventType())
      {
         case osgGA::GUIEventAdapter::FRAME:
            handleFrameEvent (ea);
            break;

         case osgGA::GUIEventAdapter::PUSH:
            handlePushEvent (ea);
            break;

         case osgGA::GUIEventAdapter::RELEASE:
            handleReleaseEvent (ea);
            break;

         case osgGA::GUIEventAdapter::KEYDOWN:
            handleKeyDownEvent (ea);
            break;

         case osgGA::GUIEventAdapter::KEYUP:
            handleKeyUpEvent (ea);
            break;

         case osgGA::GUIEventAdapter::SCROLL:
            handleScrollEvent (ea);
            break;

         default:
            break;
      }

      kbdFocusPolicy_->updateFocus (ea, nodeUnderMouse_);
      wheelFocusPolicy_->updateFocus (ea, nodeUnderMouse_);

      return handleReturnValues_[ea.getEventType()];
   }



   // - EventHandler::setPickingRoots ------------------------------------------
   void EventHandler::setPickingRoots (std::vector<NodePtr> newRoots)
   {
      pickingRoots_ = newRoots;
   }



   // - EventHandler::setPickingRoot -------------------------------------------
   void EventHandler::setPickingRoot (NodePtr newRoot)
   {
      std::vector<NodePtr> newRoots;
      newRoots.push_back (newRoot);
      setPickingRoots (newRoots);
   }



   // - EventHandler::addNode --------------------------------------------------
   void EventHandler::addNode (const osg::ref_ptr<osg::Node> node)
   {
#     define OSGUISH_EVENTHANDLER_ADD_EVENT(EVENT) \
         signals_[node][#EVENT] = SignalPtr (new EventHandler::Signal_t());

      OSGUISH_EVENTHANDLER_ADD_EVENT (MouseEnter);
      OSGUISH_EVENTHANDLER_ADD_EVENT (MouseLeave);
      OSGUISH_EVENTHANDLER_ADD_EVENT (MouseMove);
      OSGUISH_EVENTHANDLER_ADD_EVENT (MouseDown);
      OSGUISH_EVENTHANDLER_ADD_EVENT (MouseUp);
      OSGUISH_EVENTHANDLER_ADD_EVENT (Click);
      OSGUISH_EVENTHANDLER_ADD_EVENT (DoubleClick);
      OSGUISH_EVENTHANDLER_ADD_EVENT (MouseWheelUp);
      OSGUISH_EVENTHANDLER_ADD_EVENT (MouseWheelDown);
      OSGUISH_EVENTHANDLER_ADD_EVENT (KeyUp);
      OSGUISH_EVENTHANDLER_ADD_EVENT (KeyDown);

#     undef OSGUISH_EVENTHANDLER_ADD_EVENT
   }


   // - EventHandler::getSignal ------------------------------------------------
   EventHandler::SignalPtr EventHandler::getSignal(
      const NodePtr node, const std::string& signal)
   {
       SignalsMap_t::const_iterator signalsCollectionIter =
          signals_.find (node);

      if (signalsCollectionIter == signals_.end())
      {
         throw std::runtime_error(
            ("Trying to get a signal of an unknown node: '" + node->getName()
             + "' (" + boost::lexical_cast<std::string>(node.get())
             + ").").c_str());
      }

      SignalCollection_t::const_iterator signalIter =
         signalsCollectionIter->second.find (signal);

      if (signalIter == signalsCollectionIter->second.end())
      {
         throw std::runtime_error (("Trying to get an unknown signal: '"
                                    + signal + "'.").c_str());
      }

      return signalIter->second;
   }



   // - EventHandler::setKeyboardFocus -----------------------------------------
   void EventHandler::setKeyboardFocus (const NodePtr node)
   {
      kbdFocus_ = node;
   }



   // - EventHandler::setMouseWheelFocus ---------------------------------------
   void EventHandler::setMouseWheelFocus (const NodePtr node)
   {
      wheelFocus_ = node;
   }



   // - EventHandler::setKeyboardFocusPolicy -----------------------------------
   void EventHandler::setKeyboardFocusPolicy(
      const FocusPolicyFactory& policyFactory)
   {
      kbdFocusPolicy_ = policyFactory.create (kbdFocus_);
   }



   // - EventHandler::setMouseWheelFocusPolicy ---------------------------------
   void EventHandler::setMouseWheelFocusPolicy(
      const FocusPolicyFactory& policyFactory)
   {
      wheelFocusPolicy_ = policyFactory.create (wheelFocus_);
   }



   // - EventHandler::getObservedNode ------------------------------------------
   NodePtr EventHandler::getObservedNode (const osg::NodePath& nodePath)
   {
      typedef osg::NodePath::const_reverse_iterator iter_t;
      for (iter_t p = nodePath.rbegin(); p != nodePath.rend(); ++p)
      {
         if (signals_.find (NodePtr(*p)) != signals_.end())
            return NodePtr(*p);
      }

      return NodePtr();
   }



   // - EventHandler::handleFrameEvent -----------------------------------------
   void EventHandler::handleFrameEvent (const osgGA::GUIEventAdapter& ea)
   {
      assert (pickingRoots_.size() > 0);

      // Find out who is, and who was under the mouse pointer
      NodePtr currentNodeUnderMouse;
      osg::Vec3 currentPositionUnderMouse;

      typedef std::vector <NodePtr>::iterator iter_t;
      for (iter_t p = pickingRoots_.begin(); p != pickingRoots_.end(); ++p)
      {
         osgUtil::IntersectVisitor::HitList hitList;
         viewer_.computeIntersections (ea.getXnormalized(), ea.getYnormalized(),
                                       p->get(), hitList);

         if (hitList.size() > 0)
         {
            typedef osgUtil::IntersectVisitor::HitList::const_iterator hl_iter_t;
            hl_iter_t theHit = hitList.end();

            if (ignoreBackFaces_)
            {
               for (hl_iter_t hit = hitList.begin(); hit != hitList.end(); ++hit)
               {
                  osg::Vec3 localVec = hit->getLocalLineSegment()->end()
                     - hit->getLocalLineSegment()->start();
                  localVec.normalize();

                  const bool frontFacing =
                     localVec * hit->getLocalIntersectNormal() < 0.0;

                  if (frontFacing)
                  {
                     theHit = hit;
                     break;
                  }
               }
            }
            else // !ignoreBackFaces_
               theHit = hitList.begin();

            if (theHit != hitList.end())
            {
               currentNodeUnderMouse = getObservedNode (theHit->getNodePath());
               assert (signals_.find (currentNodeUnderMouse) != signals_.end()
                       && "'getObservedNode()' returned an invalid value!");

               currentPositionUnderMouse = theHit->getLocalIntersectPoint();

               hitUnderMouse_ = *theHit;

               break;
            }
         }  // if (hitList.size() > 0)
      } // for (...pickingRoots_...)

      NodePtr prevNodeUnderMouse = nodeUnderMouse_;
      osg::Vec3 prevPositionUnderMouse = positionUnderMouse_;

      nodeUnderMouse_ = currentNodeUnderMouse;
      positionUnderMouse_ = currentPositionUnderMouse;

      // Trigger the events
      if (currentNodeUnderMouse == prevNodeUnderMouse)
      {
         if (prevNodeUnderMouse.valid()
             && currentPositionUnderMouse != prevPositionUnderMouse)
         {
            signals_[currentNodeUnderMouse]["MouseMove"]->operator()(
               currentNodeUnderMouse, ea, hitUnderMouse_);
         }
      }
      else // currentNodeUnderMouse != prevNodeUnderMouse
      {
         if (prevNodeUnderMouse.valid())
         {
            signals_[prevNodeUnderMouse]["MouseLeave"]->operator()(
               prevNodeUnderMouse, ea, hitUnderMouse_);
         }

         if (currentNodeUnderMouse.valid())
         {
            signals_[currentNodeUnderMouse]["MouseEnter"]->operator()(
               currentNodeUnderMouse, ea, hitUnderMouse_);
         }
      }
   }



   // - EventHandler::handlePushEvent ------------------------------------------
   void EventHandler::handlePushEvent (const osgGA::GUIEventAdapter& ea)
   {
      // Trigger a "MouseDown" signal.
      if (nodeUnderMouse_.valid())
      {
         signals_[nodeUnderMouse_]["MouseDown"]->operator()(
            nodeUnderMouse_, ea, hitUnderMouse_);
      }

      // Do the bookkeeping for "Click" and "DoubleClick"
      MouseButton button = getMouseButton (ea);
      nodeThatGotMouseDown_[button] = nodeUnderMouse_;
   }



   // - EventHandler::handleReleaseEvent ---------------------------------------
   void EventHandler::handleReleaseEvent (const osgGA::GUIEventAdapter& ea)
   {
      const double DOUBLE_CLICK_INTERVAL = 0.3;

      if (nodeUnderMouse_.valid())
      {
         MouseButton button = getMouseButton (ea);

         // First the trivial case: the "MouseUp" event
         signals_[nodeUnderMouse_]["MouseUp"]->operator()(
            nodeUnderMouse_, ea, hitUnderMouse_);

         // Now, the trickier ones: "Click" and "DoubleClick"
         if (nodeUnderMouse_ == nodeThatGotMouseDown_[button])
         {
            signals_[nodeUnderMouse_]["Click"]->operator()(
               nodeUnderMouse_, ea, hitUnderMouse_);

            const double now = ea.getTime();

            if (now - timeOfLastClick_[button] < DOUBLE_CLICK_INTERVAL
                && nodeUnderMouse_ == nodeThatGotClick_[button])
            {
               signals_[nodeUnderMouse_]["DoubleClick"]->operator()(
                  nodeUnderMouse_, ea, hitUnderMouse_);
            }

            nodeThatGotClick_[button] = nodeUnderMouse_;
            timeOfLastClick_[button] = now;
         }
      }
   }



   // - EventHandler::handleKeyDownEvent ---------------------------------------
   void EventHandler::handleKeyDownEvent (const osgGA::GUIEventAdapter& ea)
   {
      signals_[kbdFocus_]["KeyDown"]->operator()(kbdFocus_, ea, hitUnderMouse_);
   }



   // - EventHandler::handleKeyUpEvent -----------------------------------------
   void EventHandler::handleKeyUpEvent (const osgGA::GUIEventAdapter& ea)
   {
      signals_[kbdFocus_]["KeyUp"]->operator()(kbdFocus_, ea, hitUnderMouse_);
   }



   // - EventHandler::handleScrollEvent ----------------------------------------
   void EventHandler::handleScrollEvent (const osgGA::GUIEventAdapter& ea)
   {
      switch (ea.getScrollingMotion())
      {
         case osgGA::GUIEventAdapter::SCROLL_UP:
         {
            signals_[wheelFocus_]["MouseWheelUp"]->operator()(wheelFocus_, ea,
                                                              hitUnderMouse_);
            break;
         }

         case osgGA::GUIEventAdapter::SCROLL_DOWN:
         {
            signals_[wheelFocus_]["MouseWheelDown"]->operator()(wheelFocus_, ea,
                                                                hitUnderMouse_);
            break;
         }

         default:
            break; // ignore other events
      }
   }



   // - EventHandler::getMouseButton -------------------------------------------
   EventHandler::MouseButton EventHandler::getMouseButton(
      const osgGA::GUIEventAdapter& ea)
   {
      switch (ea.getButton())
      {
         case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON:
            return LEFT_MOUSE_BUTTON;
         case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON:
            return MIDDLE_MOUSE_BUTTON;
         case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON:
            return RIGHT_MOUSE_BUTTON;
         default:
         {
            assert (false && "Got an invalid mouse button code. Is 'ea' really "
                    "a mouse event?");
         }
      }
   }

} // namespace OSGUIsh
