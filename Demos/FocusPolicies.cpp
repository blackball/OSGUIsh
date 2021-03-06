/******************************************************************************\
* FocusPolicies.cpp                                                            *
* Showcase for the various focus policies for keyboard and mouse wheel events. *
* Leandro Motta Barros                                                         *
\******************************************************************************/

#include <iostream>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgText/Text>
#include <OSGUIsh/EventHandler.hpp>
#include <OSGUIsh/MouseOverFocusPolicy.hpp>
#include <OSGUIsh/MouseDownFocusPolicy.hpp>

//
// The available focus policies
//
enum FocusPolicy
{
   ManualFocusPolicy,
   MouseOverFocusPolicy,
   MouseDownFocusPolicy
};



//
// Some globals (globals are not a problem in simple examples ;-))
//
OSGUIsh::NodePtr NodeUnderMouse;

osg::ref_ptr<osg::Node> TreeNode;
osg::ref_ptr<osg::Node> StrawberryNode;
osg::ref_ptr<osg::Node> FishNode;
osg::ref_ptr<osgText::Text> TextMouseOver;
osg::ref_ptr<osgText::Text> TextMouseWheelEvent;
osg::ref_ptr<osgText::Text> TextKeyboardEvent;
osg::ref_ptr<osgText::Text> TextKeyboardFocusPolicy;
osg::ref_ptr<osgText::Text> TextMouseWheelFocusPolicy;

FocusPolicy KeyboardFocusPolicy = ManualFocusPolicy;
FocusPolicy MouseWheelFocusPolicy = ManualFocusPolicy;



//
// The event handlers
//

void HandleMouseEnter(OSGUIsh::HandlerParams& params)
{
   TextMouseOver->setText("Mouse over " + params.node->getName());
}

void HandleMouseLeave(OSGUIsh::HandlerParams& params)
{
   TextMouseOver->setText("Mouse over nothing!");
}

void HandleMouseWheelUp(OSGUIsh::HandlerParams& params)
{
   TextMouseWheelEvent->setText(params.node->getName() + ": Mouse wheel up!");
}

void HandleMouseWheelDown(OSGUIsh::HandlerParams& params)
{
   TextMouseWheelEvent->setText(params.node->getName() + ": Mouse wheel down!");
}

void HandleKeyDown (OSGUIsh::HandlerParams& params)
{
   TextKeyboardEvent->setText(params.node->getName() + ": key down: "
                              + static_cast<char>(params.event.getKey()));
}

void HandleKeyUp (OSGUIsh::HandlerParams& params)
{
   TextKeyboardEvent->setText(params.node->getName() + ": key up: "
                              + static_cast<char>(params.event.getKey()));
}



//
// ChangePolicyEventHandler
//
class ChangePolicyEventHandler: public osgGA::GUIEventHandler
{
   public:
      ChangePolicyEventHandler(osg::ref_ptr<OSGUIsh::EventHandler> eh)
         : eh_(eh)
      {
         // empty...
      }

      virtual bool handle(const osgGA::GUIEventAdapter& ea,
                          osgGA::GUIActionAdapter&)
      {
         if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
         {
            if (ea.getKey() == 'm' || ea.getKey() == 'M')
            {
               switch (MouseWheelFocusPolicy)
               {
                  case ManualFocusPolicy:
                     eh_->setMouseWheelFocusPolicy(
                        OSGUIsh::FocusPolicyFactoryMason<OSGUIsh::MouseOverFocusPolicy>());
                     MouseWheelFocusPolicy = MouseOverFocusPolicy;
                     TextMouseWheelFocusPolicy->setText(
                        "Mouse wheel focus policy: mouse over sets focus");
                     break;
                  case MouseOverFocusPolicy:
                     eh_->setMouseWheelFocusPolicy(
                        OSGUIsh::FocusPolicyFactoryMason<
                           OSGUIsh::MouseDownFocusPolicy>());
                     MouseWheelFocusPolicy = MouseDownFocusPolicy;
                     TextMouseWheelFocusPolicy->setText(
                        "Mouse wheel focus policy: mouse down sets focus");
                     break;
                  case MouseDownFocusPolicy:
                     eh_->setMouseWheelFocusPolicy(
                        OSGUIsh::FocusPolicyFactoryMason<
                           OSGUIsh::ManualFocusPolicy>());
                     TextMouseWheelFocusPolicy->setText(
                        std::string("Mouse wheel focus policy: manual focus ")
                        + "change (don't change focus)");
                     MouseWheelFocusPolicy = ManualFocusPolicy;
                     break;
               }
            }
            else if (ea.getKey() == 'k' || ea.getKey() == 'K')
            {
               switch (KeyboardFocusPolicy)
               {
                  case ManualFocusPolicy:
                     eh_->setKeyboardFocusPolicy(
                        OSGUIsh::FocusPolicyFactoryMason<
                           OSGUIsh::MouseOverFocusPolicy>());
                     KeyboardFocusPolicy = MouseOverFocusPolicy;
                     TextKeyboardFocusPolicy->setText(
                        "Keyboard focus policy: mouse over sets focus");
                     break;
                  case MouseOverFocusPolicy:
                     eh_->setKeyboardFocusPolicy(
                        OSGUIsh::FocusPolicyFactoryMason<
                           OSGUIsh::MouseDownFocusPolicy>());
                     KeyboardFocusPolicy = MouseDownFocusPolicy;
                     TextKeyboardFocusPolicy->setText(
                        "Keyboard focus policy: mouse down sets focus");
                     break;
                  case MouseDownFocusPolicy:
                     eh_->setKeyboardFocusPolicy(
                        OSGUIsh::FocusPolicyFactoryMason<
                           OSGUIsh::ManualFocusPolicy>());
                     KeyboardFocusPolicy = ManualFocusPolicy;
                     TextKeyboardFocusPolicy->setText(
                        std::string("Keyboard focus policy: manual focus ")
                        + "change (don't change focus)");
                     break;
               }
            }
         }
         return false;
      }

   private:
      osg::ref_ptr<OSGUIsh::EventHandler> eh_;
};



// - CreateHUD -----------------------------------------------------------------
osg::ref_ptr<osg::Projection> CreateHUD(int width, int height)
{
   // Create the text nodes to be displayed on the HUD
   osg::ref_ptr<osg::Geode> hudGeometry(new osg::Geode());

   TextMouseOver = new osgText::Text();
   TextMouseOver->setDataVariance(osg::Object::DYNAMIC);
   TextMouseOver->setText("Mouse over nothing!");
   TextMouseOver->setFont("Data/bluehigl.ttf");
   TextMouseOver->setPosition(osg::Vec3 (10.0f, 10.0f, 0.0f));
   TextMouseOver->setCharacterSize(25.0);
   hudGeometry->addDrawable(TextMouseOver);

   TextMouseWheelEvent = new osgText::Text();
   TextMouseWheelEvent->setDataVariance(osg::Object::DYNAMIC);
   TextMouseWheelEvent->setText("Waiting for mouse wheel event");
   TextMouseWheelEvent->setFont("Data/bluehigl.ttf");
   TextMouseWheelEvent->setPosition(osg::Vec3 (10.0f, 40.0f, 0.0f));
   TextMouseWheelEvent->setCharacterSize(25.0);
   hudGeometry->addDrawable(TextMouseWheelEvent);

   TextKeyboardEvent = new osgText::Text();
   TextKeyboardEvent->setDataVariance(osg::Object::DYNAMIC);
   TextKeyboardEvent->setText("Waiting for keyboard event");
   TextKeyboardEvent->setFont("Data/bluehigl.ttf");
   TextKeyboardEvent->setPosition(osg::Vec3 (10.0f, 70.0f, 0.0f));
   TextKeyboardEvent->setCharacterSize(25.0);
   hudGeometry->addDrawable(TextKeyboardEvent);

   TextMouseWheelFocusPolicy = new osgText::Text();
   TextMouseWheelFocusPolicy->setDataVariance(osg::Object::DYNAMIC);
   TextMouseWheelFocusPolicy->setText(
      "Mouse wheel focus policy: mouse down sets focus");
   TextMouseWheelFocusPolicy->setFont("Data/bluehigl.ttf");
   TextMouseWheelFocusPolicy->setPosition(osg::Vec3(10.0f, 100.0f, 0.0f));
   TextMouseWheelFocusPolicy->setCharacterSize(25.0);
   hudGeometry->addDrawable(TextMouseWheelFocusPolicy);

   TextKeyboardFocusPolicy = new osgText::Text();
   TextKeyboardFocusPolicy->setDataVariance(osg::Object::DYNAMIC);
   TextKeyboardFocusPolicy->setText(
      "Keyboard focus policy: mouse over sets focus");
   TextKeyboardFocusPolicy->setFont("Data/bluehigl.ttf");
   TextKeyboardFocusPolicy->setPosition(osg::Vec3 (10.0f, 130.0f, 0.0f));
   TextKeyboardFocusPolicy->setCharacterSize(25.0);
   hudGeometry->addDrawable(TextKeyboardFocusPolicy);

   // Create the HUD per se
   osg::ref_ptr<osg::StateSet> stateSet = hudGeometry->getOrCreateStateSet();
   stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
   stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
   stateSet->setRenderBinDetails(11, "RenderBin");

   osg::ref_ptr<osg::MatrixTransform> modelviewAbs(new osg::MatrixTransform);
   modelviewAbs->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
   modelviewAbs->setMatrix(osg::Matrix::identity());

   modelviewAbs->addChild(hudGeometry);

   osg::ref_ptr<osg::Projection> projection(new osg::Projection());
   projection->setMatrix(osg::Matrix::ortho2D(0, width, 0, height));
   projection->addChild(modelviewAbs);

   return projection;
}



// - LoadModel -----------------------------------------------------------------
osg::ref_ptr<osg::Node> LoadModel(const std::string& fileName)
{
   osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile(fileName);

   if (!loadedModel)
   {
      std::cerr << "Problem opening '" << fileName << "'\n";
      exit(1);
   }

   return loadedModel;
}



// - LoadModels ----------------------------------------------------------------
osg::ref_ptr<osg::Group> LoadModels()
{
   osg::ref_ptr<osg::Group> group(new osg::Group);

   TreeNode = LoadModel("Data/Tree_01.3ds");
   TreeNode->setName("Tree");
   osg::ref_ptr<osg::PositionAttitudeTransform> treePAT(
      new osg::PositionAttitudeTransform());
   treePAT->addChild(TreeNode);
   treePAT->setPosition(osg::Vec3(1.2, 0.0, 0.0));
   group->addChild(treePAT);

   StrawberryNode = LoadModel("Data/Strawberry.3ds");
   StrawberryNode->setName("Strawberry");
   osg::ref_ptr<osg::PositionAttitudeTransform> strawberryPAT(
      new osg::PositionAttitudeTransform());
   strawberryPAT->addChild(StrawberryNode);
   strawberryPAT->setPosition(osg::Vec3(-1.0, 0.0, 0.0));
   group->addChild(strawberryPAT);

   FishNode = LoadModel("Data/Fish.3ds");
   FishNode->setName("Fish");

   group->addChild(FishNode);

   return group;
}



// - main ----------------------------------------------------------------------
int main(int argc, char* argv[])
{
   // Create viewer
   osgViewer::Viewer viewer;
   viewer.setUpViewInWindow(0, 0, 1024, 768);

   // Construct the scene graph, set it as the data to be viewed
   osg::ref_ptr<osg::Group> sgRoot = LoadModels();
   sgRoot->addChild(CreateHUD(1024, 768));
   viewer.setSceneData(sgRoot);

   // Create the OSGUIsh event handler
   osg::ref_ptr<OSGUIsh::EventHandler> guishEH(
      new OSGUIsh::EventHandler(
         0.0,
         OSGUIsh::FocusPolicyFactoryMason<OSGUIsh::MouseOverFocusPolicy>(),
         OSGUIsh::FocusPolicyFactoryMason<OSGUIsh::MouseDownFocusPolicy>()));

   viewer.addEventHandler(guishEH);

   // Add an event handler for changing the policies
   osg::ref_ptr<ChangePolicyEventHandler> focusPolicyEH(
      new ChangePolicyEventHandler(guishEH));

   viewer.addEventHandler(focusPolicyEH);

   // Adds the node to the event handler, so that it can get events
   guishEH->addNode(TreeNode);
   guishEH->addNode(StrawberryNode);
   guishEH->addNode(FishNode);

   // Register event handlers
   guishEH->getSignal(TreeNode, OSGUIsh::EVENT_MOUSE_ENTER)
      ->connect(&HandleMouseEnter);
   guishEH->getSignal(StrawberryNode, OSGUIsh::EVENT_MOUSE_ENTER)
      ->connect(&HandleMouseEnter);
   guishEH->getSignal(FishNode, OSGUIsh::EVENT_MOUSE_ENTER)
      ->connect(&HandleMouseEnter);

   guishEH->getSignal(TreeNode, OSGUIsh::EVENT_MOUSE_LEAVE)
      ->connect(&HandleMouseLeave);
   guishEH->getSignal(StrawberryNode, OSGUIsh::EVENT_MOUSE_LEAVE)
      ->connect(&HandleMouseLeave);
   guishEH->getSignal(FishNode, OSGUIsh::EVENT_MOUSE_LEAVE)
      ->connect(&HandleMouseLeave);

   guishEH->getSignal(TreeNode, OSGUIsh::EVENT_MOUSE_WHEEL_UP)
      ->connect(&HandleMouseWheelUp);
   guishEH->getSignal(StrawberryNode, OSGUIsh::EVENT_MOUSE_WHEEL_UP)
      ->connect(&HandleMouseWheelUp);
   guishEH->getSignal(FishNode, OSGUIsh::EVENT_MOUSE_WHEEL_UP)
      ->connect(&HandleMouseWheelUp);

   guishEH->getSignal(TreeNode, OSGUIsh::EVENT_MOUSE_WHEEL_DOWN)
      ->connect(&HandleMouseWheelDown);
   guishEH->getSignal(StrawberryNode, OSGUIsh::EVENT_MOUSE_WHEEL_DOWN)
      ->connect(&HandleMouseWheelDown);
   guishEH->getSignal(FishNode, OSGUIsh::EVENT_MOUSE_WHEEL_DOWN)
      ->connect(&HandleMouseWheelDown);

   guishEH->getSignal(TreeNode, OSGUIsh::EVENT_KEY_DOWN)
      ->connect(&HandleKeyDown);
   guishEH->getSignal(StrawberryNode, OSGUIsh::EVENT_KEY_DOWN)
      ->connect(&HandleKeyDown);
   guishEH->getSignal(FishNode, OSGUIsh::EVENT_KEY_DOWN)
      ->connect(&HandleKeyDown);

   // Enter rendering loop
   viewer.run();
}
