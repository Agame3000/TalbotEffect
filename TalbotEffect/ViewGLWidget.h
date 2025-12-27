#pragma once

#include "UniformDataStructure.h"
class MainWindow;
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

class ViewGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT
public:
	enum Type {Vertical, Horizontal};

	ViewGLWidget(QWidget* parent, UniformDataStructure* uniStruct, MainWindow* win, Type type) : QOpenGLWidget(parent), uniStruct(uniStruct), type(type), win(win) {}

	bool ready = true;
	float time = 0;
protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;
	void paintEvent(QPaintEvent* event) override;
private:
	MainWindow* win;
	Type type;
	QOpenGLShaderProgram* program = nullptr;
	UniformDataStructure* uniStruct;
	GLint posAttrID = 0;
	GLint resUniID = 0;
	std::vector<GLint> uniformIDs;
	GLint anUniID = 0;
};