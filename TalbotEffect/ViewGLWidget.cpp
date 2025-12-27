#include "ViewGLWidget.h"
#include "MainWindow.h"
#include <QResource>
#include <QMetaMethod>
#include <QPainter>
#include <chrono>
#include <ctime>

using namespace std::chrono;

const GLfloat normVertices[] = {
		-1.0f,  1.0f,
		1.0f, 1.0f,
		-1.0f, -1.0f,
		1.0f, -1.0f
};

void ViewGLWidget::initializeGL() {
	initializeOpenGLFunctions();
	glClearColor(0, 0, 0, 1);

	program = new QOpenGLShaderProgram(this);
	QResource rVert(":/shaders/res/shaders/vert.glsl");
	program->addShaderFromSourceCode(QOpenGLShader::Vertex,
		QString::fromUtf8(rVert.uncompressedData()));
	const char* fsName = nullptr;
	switch (type)
	{
	case ViewGLWidget::Vertical:
		fsName = ":/shaders/res/shaders/frag_v.glsl";
		break;
	case ViewGLWidget::Horizontal:
		fsName = ":/shaders/res/shaders/frag_h.glsl";
		break;
	default:
		Q_ASSERT(true);
	}
	QResource rFrag(fsName);
	program->addShaderFromSourceCode(QOpenGLShader::Fragment,
		QString::fromUtf8(rFrag.uncompressedData()));
	program->link();

	posAttrID = program->attributeLocation("posAttr");
	Q_ASSERT(posAttrID != -1);
	resUniID = program->uniformLocation("iRes");
	Q_ASSERT(resUniID != -1);
	uniStruct->forEach([&](UniformDataComponent* c, int i) {
		GLint id = program->uniformLocation(c->id);
		Q_ASSERT(id != -1);
		uniformIDs.push_back(id);
		}
	);
	anUniID = program->uniformLocation("an");
}

void ViewGLWidget::resizeGL(int w, int h) {
	ready = false;
	auto start = high_resolution_clock::now();
	win->updateWindowStatus();

	program->bind();
	program->setUniformValue(resUniID, QVector2D(w, h));
	program->release();

	ready = true;
	time = duration_cast<nanoseconds>(high_resolution_clock::now() - start).count() / 1000.0f;
	win->updateWindowStatus();
}

void ViewGLWidget::paintGL() {
	ready = false;
	auto start = high_resolution_clock::now();
	win->updateWindowStatus();

	glClear(GL_COLOR_BUFFER_BIT);

	program->bind();

	uniStruct->forEach([&](UniformDataComponent* c, int i) {
		switch (c->type)
		{
		case QMetaType::Type::Float:
			program->setUniformValue(c->id, (float)*c);
			break;
		case QMetaType::Type::Int:
			program->setUniformValue(c->id, (int)*c);
			break;
		case QMetaType::Type::QVector2D:
			program->setUniformValue(c->id, c->cast<QVector2D>());
			break;
		case QMetaType::Type::QVector3D:
			program->setUniformValue(c->id, c->cast<QVector3D>());
			break;
		default:
			Q_ASSERT(true);
		}
		}
	);

	program->setUniformValueArray(anUniID, win->an->data(), win->an->size());

	glVertexAttribPointer(posAttrID, 2, GL_FLOAT, GL_FALSE, 0, normVertices);
	glEnableVertexAttribArray(posAttrID);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(posAttrID);

	program->release();

	ready = true;
	time = duration_cast<nanoseconds>(high_resolution_clock::now() - start).count() / 1000.0f;
	win->updateWindowStatus();
}

void ViewGLWidget::paintEvent(QPaintEvent* event) {
	QOpenGLWidget::paintEvent(event);
	QPainter painter(this);

	if (type == Type::Vertical) {
		painter.setPen(QPen(QColor(255, 0, 255, 167), 3));
		int x = 1 + (float)*win->z / (float)*win->viewSizeX * (width() - 1);
		painter.drawLine(x, 0, x, height());
	}
}