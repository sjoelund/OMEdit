/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-2014, Open Source Modelica Consortium (OSMC),
 * c/o Linköpings universitet, Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 LICENSE OR
 * THIS OSMC PUBLIC LICENSE (OSMC-PL) VERSION 1.2.
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES
 * RECIPIENT'S ACCEPTANCE OF THE OSMC PUBLIC LICENSE OR THE GPL VERSION 3,
 * ACCORDING TO RECIPIENTS CHOICE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from OSMC, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or
 * http://www.openmodelica.org, and in the OpenModelica distribution.
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */
/*
 * @author Volker Waurich <volker.waurich@tu-dresden.de>
 */

#include "AnimationWindow.h"
#include <osg/Version>

/*!
  \class AnimationWindow
  \brief A QMainWindow for animation.
  */
/*!
 * \brief AnimationWindow::AnimationWindow
 * \param pPlotWindowContainer
 */
AnimationWindow::AnimationWindow(PlotWindowContainer *pPlotWindowContainer)
  : QMainWindow(pPlotWindowContainer),
    osgViewer::CompositeViewer(),
    mpPlotWindowContainer(pPlotWindowContainer),
    mPathName(""),
    mFileName(""),
    mpSceneView(new osgViewer::View()),
    mpVisualizer(nullptr),
    mpViewerWidget(nullptr),
    mpUpdateTimer(new QTimer()),
    mpAnimationToolBar(new QToolBar(QString("Animation Toolbar"),this)),
    mpAnimationChooseFileAction(nullptr),
    mpAnimationInitializeAction(nullptr),
    mpAnimationPlayAction(nullptr),
    mpAnimationPauseAction(nullptr)
{
  // to distinguish this widget as a subwindow among the plotwindows
  this->setObjectName(QString("animationWidget"));
  // the osg threading model
  setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);
  //the viewer widget
  mpViewerWidget = setupViewWidget();
  // we need to set the minimum height so that visualization window is still shown when we cascade windows.
  mpViewerWidget->setMinimumHeight(100);
  // let timer do a scene update at every tick
  QObject::connect(mpUpdateTimer, SIGNAL(timeout()), this, SLOT(updateSceneFunction()));
  QObject::connect(mpUpdateTimer, SIGNAL(timeout()), this, SLOT(renderSlotFunction()));
  mpUpdateTimer->start(100);
  // actions and widgets for the toolbar
  mpAnimationChooseFileAction = new QAction(QIcon(":/Resources/icons/openFile.png"), Helper::animationChooseFile, this);
  mpAnimationChooseFileAction->setStatusTip(Helper::animationChooseFileTip);
  mpAnimationInitializeAction = new QAction(QIcon(":/Resources/icons/initialize.png"), Helper::animationInitialize, this);
  mpAnimationInitializeAction->setStatusTip(Helper::animationInitializeTip);
  mpAnimationInitializeAction->setEnabled(false);
  mpAnimationPlayAction = new QAction(QIcon(":/Resources/icons/play.png"), Helper::animationPlay, this);
  mpAnimationPlayAction->setStatusTip(Helper::animationPlayTip);
  mpAnimationPlayAction->setEnabled(false);
  mpAnimationPauseAction = new QAction(QIcon(":/Resources/icons/pause.png"), Helper::animationPause, this);
  mpAnimationPauseAction->setStatusTip(Helper::animationPauseTip);
  mpAnimationPauseAction->setEnabled(false);
  mpAnimationSlider = new QSlider(Qt::Horizontal);
  mpAnimationSlider->setMinimum(0);
  mpAnimationSlider->setMaximum(100);
  mpAnimationSlider->setSliderPosition(50);
  mpAnimationSlider->setEnabled(false);
  mpAnimationTimeLabel = new QLabel();
  mpAnimationTimeLabel->setText(QString(" Time [s]: ").append(QString::fromStdString("0.000")));
  mpPerspectiveDropDownBox = new QComboBox(this);
  mpPerspectiveDropDownBox->addItem(QIcon(":/Resources/icons/perspective0.png"), QString("to home position"));
  mpPerspectiveDropDownBox->addItem(QIcon(":/Resources/icons/perspective2.png"),QString("normal to x-y plane"));
  mpPerspectiveDropDownBox->addItem(QIcon(":/Resources/icons/perspective1.png"),QString("normal to y-z plane"));
  mpPerspectiveDropDownBox->addItem(QIcon(":/Resources/icons/perspective3.png"),QString("normal to x-z plane"));
  //assemble the animation toolbar
  mpAnimationToolBar->addAction(mpAnimationChooseFileAction);
  mpAnimationToolBar->addSeparator();
  mpAnimationToolBar->addAction(mpAnimationInitializeAction);
  mpAnimationToolBar->addSeparator();
  mpAnimationToolBar->addAction(mpAnimationPlayAction);
  mpAnimationToolBar->addSeparator();
  mpAnimationToolBar->addAction(mpAnimationPauseAction);
  mpAnimationToolBar->addSeparator();
  mpAnimationToolBar->addWidget(mpAnimationSlider);
  mpAnimationToolBar->addWidget(mpAnimationTimeLabel);
  mpAnimationToolBar->addWidget(mpPerspectiveDropDownBox);
  int toolbarIconSize = mpPlotWindowContainer->getMainWindow()->getOptionsDialog()->getGeneralSettingsPage()->getToolbarIconSizeSpinBox()->value();
  mpAnimationToolBar->setIconSize(QSize(toolbarIconSize, toolbarIconSize));
  addToolBar(Qt::TopToolBarArea,mpAnimationToolBar);
  setCentralWidget(mpViewerWidget);
  //connections
  connect(mpAnimationChooseFileAction, SIGNAL(triggered()),this, SLOT(chooseAnimationFileSlotFunction()));
  connect(mpAnimationInitializeAction, SIGNAL(triggered()),this, SLOT(initSlotFunction()));
  connect(mpAnimationPlayAction, SIGNAL(triggered()),this, SLOT(playSlotFunction()));
  connect(mpAnimationPauseAction, SIGNAL(triggered()),this, SLOT(pauseSlotFunction()));
  connect(mpPerspectiveDropDownBox, SIGNAL(activated(int)), this, SLOT(setPerspective(int)));
  connect(mpAnimationSlider, SIGNAL(sliderMoved(int)),this, SLOT(sliderSetTimeSlotFunction(int)));
}

/*!
 * \brief AnimationWindow::setupViewWidget
 * creates the widget for the osg viewer
 * \return the widget
 */
QWidget* AnimationWindow::setupViewWidget()
{
  //desktop resolution
  QRect rec = QApplication::desktop()->screenGeometry();
  int height = rec.height();
  int width = rec.width();
  //int height = 1000;
  //int width = 2000;
  //get context
  osg::ref_ptr<osg::DisplaySettings> ds = osg::DisplaySettings::instance().get();
  osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits();
  traits->windowName = "";
  traits->windowDecoration = false;
  traits->x = 0;
  traits->y = 0;
  traits->width = width;
  traits->height = height;
  traits->doubleBuffer = true;
  traits->alpha = ds->getMinimumNumAlphaBits();
  traits->stencil = ds->getMinimumNumStencilBits();
  traits->sampleBuffers = ds->getMultiSamples();
  traits->samples = ds->getNumMultiSamples();
  osg::ref_ptr<osgQt::GraphicsWindowQt> gw = new osgQt::GraphicsWindowQt(traits.get());
  //add a scene to viewer
  addView(mpSceneView);
  //get the viewer widget
  osg::ref_ptr<osg::Camera> camera = mpSceneView->getCamera();
  camera->setGraphicsContext(gw);
  camera->setClearColor(osg::Vec4(0.2, 0.2, 0.6, 1.0));
  //camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
  camera->setViewport(new osg::Viewport(0, 0, width, height));
  camera->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(traits->width/2) / static_cast<double>(traits->height/2), 1.0f, 10000.0f);
  mpSceneView->addEventHandler(new osgViewer::StatsHandler());
  mpSceneView->setCameraManipulator(new osgGA::MultiTouchTrackballManipulator());
#if OPENSCENEGRAPH_MAJOR_VERSION>=3 && OPENSCENEGRAPH_MINOR_VERSION>=4
  gw->setTouchEventsEnabled(true);
#endif
  return gw->getGLWidget();
}

/*!
 * \brief AnimationWindow::loadVisualization
 * loads the data and the xml scene description
 */
void AnimationWindow::loadVisualization()
{
  VisType visType = VisType::NONE;
  // Get visualization type.
  if (isFMU(mFileName)) {
    visType = VisType::FMU;
  } else if (isMAT(mFileName)) {
    visType = VisType::MAT;
  } else {
    std::cout<<"unknown visualization type. "<<std::endl;
  }
  //init visualizer
  if (visType == VisType::MAT) {
    mpVisualizer = new VisualizerMAT(mFileName, mPathName);
  } else {
    std::cout<<"could not init "<<mPathName<<mFileName<<std::endl;
  }
  //load the XML File, build osgTree, get initial values for the shapes
  bool xmlExists = checkForXMLFile(mFileName, mPathName);
  if (!xmlExists) {
    std::cout<<"Could not find the visual XML file "<<assembleXMLFileName(mFileName, mPathName)<<std::endl;
  } else {
    mpVisualizer->initData();
    mpVisualizer->setUpScene();
    mpVisualizer->initVisualization();
    //add scene for the chosen visualization
    mpSceneView->setSceneData(mpVisualizer->getOMVisScene()->getScene().getRootNode());
  }
  //add window title
  this->setWindowTitle(QString::fromStdString(mFileName));
}

/*!
 * \brief AnimationWindow::animationFileSlotFunction
 * opens a file dialog to chooes an animation
 */
void AnimationWindow::chooseAnimationFileSlotFunction()
{
  QString *dir = new QString("./");
  std::string file = StringHandler::getOpenFileName(this, QString(Helper::applicationName).append(" - ").append(Helper::chooseFile),
      dir, Helper::matFileTypes, NULL).toStdString();
  if (file.compare("")) {
    std::size_t pos = file.find_last_of("/\\");
    mPathName = file.substr(0, pos + 1);
    mFileName = file.substr(pos + 1, file.length());
    std::cout<<"file "<<mFileName<<"   path "<<mPathName<<std::endl;
    loadVisualization();
    mpAnimationInitializeAction->setEnabled(true);
    mpAnimationPlayAction->setEnabled(true);
    mpAnimationPauseAction->setEnabled(true);
    mpAnimationSlider->setEnabled(true);
  } else {
    std::cout<<"No Visualization selected!"<<std::endl;
  }
}

/*!
 * \brief AnimationWindow::getTimeFraction
 * gets the fraction of the complete simulation time to move the slider
 */
double AnimationWindow::getTimeFraction()
{
  if (mpVisualizer==NULL) {
    return 0.0;
  } else {
    return mpVisualizer->getTimeManager()->getTimeFraction();
  }
}

/*!
 * \brief AnimationWindow::sliderSetTimeSlotFunction
 * slot function for the time slider to jump to the adjusted point of time
 */
void AnimationWindow::sliderSetTimeSlotFunction(int value)
{
  float time = (mpVisualizer->getTimeManager()->getEndTime()
                - mpVisualizer->getTimeManager()->getStartTime())
                * (float) (value / 100.0);
  mpVisualizer->getTimeManager()->setVisTime(time);
  mpVisualizer->updateScene(time);
}

/*!
 * \brief AnimationWindow::playSlotFunction
 * slot function for the play button
 */
void AnimationWindow::playSlotFunction()
{
  mpVisualizer->getTimeManager()->setPause(false);
}

/*!
 * \brief AnimationWindow::pauseSlotFunction
 * slot function for the pause button
 */
void AnimationWindow::pauseSlotFunction()
{
  mpVisualizer->getTimeManager()->setPause(true);
}

/*!
 * \brief AnimationWindow::initSlotFunction
 * slot function for the init button
 */
void AnimationWindow::initSlotFunction()
{
  mpVisualizer->initVisualization();
}

/*!
 * \brief AnimationWindow::updateSceneFunction
 * updates the visualization objects
 */
void AnimationWindow::updateSceneFunction()
{
  if (!(mpVisualizer == NULL)) {
    //update the scene
    mpVisualizer->sceneUpdate();
    // set time slider
    int time = mpVisualizer->getTimeManager()->getTimeFraction();
    mpAnimationSlider->setValue(time);
  }
}

/*!
 * \brief AnimationWindow::renderSlotFunction
 * renders the osg viewer
 */
void AnimationWindow::renderSlotFunction()
{
  frame();
}

/*!
 * \brief AnimationWindow::getVisTime
 * returns the current visualization time
 */
double AnimationWindow::getVisTime()
{
  if (mpVisualizer==NULL) {
    return -1.0;
  } else {
    return mpVisualizer->getTimeManager()->getVisTime();
  }
}

/*!
 * \brief AnimationWindow::setPathName
 * sets mpPathName
 */
void AnimationWindow::setPathName(std::string pathName)
{
  mPathName = pathName;
}

/*!
 * \brief AnimationWindow::setFileName
 * sets mpFileName
 */
void AnimationWindow::setFileName(std::string fileName)
{
  mFileName = fileName;
}

/*!
 * \brief AnimationWindow::cameraPositionXY
 * sets the camera position to XY
 */
void AnimationWindow::cameraPositionXY()
{
  resetCamera();
  //the new orientation
  osg::Matrix3 newOrient = osg::Matrix3(1, 0, 0, 0, 1, 0, 0, 0, 1);
  osg::ref_ptr<osgGA::CameraManipulator> manipulator = mpSceneView->getCameraManipulator();
  osg::Matrixd mat = manipulator->getMatrix();
  //assemble
  mat = osg::Matrixd(newOrient(0, 0), newOrient(0, 1), newOrient(0, 2), 0,
                     newOrient(1, 0), newOrient(1, 1), newOrient(1, 2), 0,
                     newOrient(2, 0), newOrient(2, 1), newOrient(2, 2), 0,
                     abs(mat(3, 0)), abs(mat(3, 2)), abs(mat(3, 1)), 1);
  manipulator->setByMatrix(mat);
}

/*!
 * \brief AnimationWindow::cameraPositionYZ
 * sets the camera position to YZ
 */
void AnimationWindow::cameraPositionYZ()
{
  //to get the correct distance of the bodies, reset to home position and use the values of this camera position
  resetCamera();
  //the new orientation
  osg::Matrix3 newOrient = osg::Matrix3(0, 1, 0, 0, 0, 1, 1, 0, 0);
  osg::ref_ptr<osgGA::CameraManipulator> manipulator = mpSceneView->getCameraManipulator();
  osg::Matrixd mat = manipulator->getMatrix();
  //assemble
  mat = osg::Matrixd(newOrient(0, 0), newOrient(0, 1), newOrient(0, 2), 0,
                     newOrient(1, 0), newOrient(1, 1), newOrient(1, 2), 0,
                     newOrient(2, 0), newOrient(2, 1), newOrient(2, 2), 0,
                     abs(mat(3, 1)), abs(mat(3, 2)), abs(mat(3, 0)), 1);
  manipulator->setByMatrix(mat);
}

/*!
 * \brief AnimationWindow::cameraPositionXZ
 * sets the camera position to XZ
 */
void AnimationWindow::cameraPositionXZ()
{
  //to get the correct distance of the bodies, reset to home position and use the values of this camera position
  resetCamera();
  //the new orientation
  osg::Matrix3 newOrient = osg::Matrix3(1, 0, 0, 0, 0, 1, 0, -1, 0);
  osg::ref_ptr<osgGA::CameraManipulator> manipulator = mpSceneView->getCameraManipulator();
  osg::Matrixd mat = manipulator->getMatrix();
  //assemble
  mat = osg::Matrixd(newOrient(0, 0), newOrient(0, 1), newOrient(0, 2), 0,
                     newOrient(1, 0), newOrient(1, 1), newOrient(1, 2), 0,
                     newOrient(2, 0), newOrient(2, 1), newOrient(2, 2), 0,
                     abs(mat(3, 0)), -abs(mat(3, 1)), abs(mat(3, 2)), 1);
  manipulator->setByMatrix(mat);
}

/*!
 * \brief AnimationWindow::resetCamera
 * resets the camera position
 */
void AnimationWindow::resetCamera()
{
  mpSceneView->home();
}

/*!
 * \brief AnimationWindow::setPerspective
 * gets the identifier for the chosen perspective and calls the functions
 */
void AnimationWindow::setPerspective(int value)
{
  switch(value) {
    case 0:
      resetCamera();
      break;
    case 1:
      cameraPositionXY();
      break;
    case 2:
      cameraPositionYZ();
      break;
    case 3:
      cameraPositionXZ();
      break;
  }
}
