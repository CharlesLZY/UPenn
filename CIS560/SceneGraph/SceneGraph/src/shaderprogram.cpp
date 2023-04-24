#include "shaderprogram.h"
#include <QFile>


ShaderProgram::ShaderProgram(OpenGLContext *context)
    : m_vertShader(), m_fragShader(), m_prog(),
      m_attrPos(-1), m_attrCol(-1),
      m_unifModel(-1), m_unifView(-1),
      context(context)
{}

void ShaderProgram::create(const char *vertfile, const char *fragfile)
{
    // Allocate space on our GPU for a vertex shader and a fragment shader and a shader program to manage the two
    m_vertShader = context->glCreateShader(GL_VERTEX_SHADER);
    m_fragShader = context->glCreateShader(GL_FRAGMENT_SHADER);
    m_prog = context->glCreateProgram();
    // Get the body of text stored in our two .glsl files
    QString qVertSource = qTextFileRead(vertfile);
    QString qFragSource = qTextFileRead(fragfile);

    char* vertSource = new char[qVertSource.size()+1];
    strcpy(vertSource, qVertSource.toStdString().c_str());
    char* fragSource = new char[qFragSource.size()+1];
    strcpy(fragSource, qFragSource.toStdString().c_str());


    // Send the shader text to OpenGL and store it in the shaders specified by the handles vertShader and fragShader
    context->glShaderSource(m_vertShader, 1, &vertSource, 0);
    context->glShaderSource(m_fragShader, 1, &fragSource, 0);
    // Tell OpenGL to compile the shader text stored above
    context->glCompileShader(m_vertShader);
    context->glCompileShader(m_fragShader);
    // Check if everything compiled OK
    GLint compiled;
    context->glGetShaderiv(m_vertShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printShaderInfoLog(m_vertShader);
    }
    context->glGetShaderiv(m_fragShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printShaderInfoLog(m_fragShader);
    }

    // Tell prog that it manages these particular vertex and fragment shaders
    context->glAttachShader(m_prog, m_vertShader);
    context->glAttachShader(m_prog, m_fragShader);
    context->glLinkProgram(m_prog);

    // Check for linking success
    GLint linked;
    context->glGetProgramiv(m_prog, GL_LINK_STATUS, &linked);
    if (!linked) {
        printLinkInfoLog(m_prog);
    }

    // Get the handles to the variables stored in our shaders
    // See shaderprogram.h for more information about these variables

    m_attrPos = context->glGetAttribLocation(m_prog, "vs_Pos");
    m_attrCol = context->glGetAttribLocation(m_prog, "vs_Col");

    m_unifModel      = context->glGetUniformLocation(m_prog, "u_Model");
    m_unifView   = context->glGetUniformLocation(m_prog, "u_View");
}

void ShaderProgram::useMe()
{
    context->glUseProgram(m_prog);
}

void ShaderProgram::setModelMatrix(const glm::mat3 &model)
{
    useMe();

    if (m_unifModel != -1)
    {
        // Pass a 3x3 matrix into a uniform variable in our shader
                        // Handle to the matrix variable on the GPU
        context->glUniformMatrix3fv(m_unifModel,
                        // How many matrices to pass
                           1,
                        // Transpose the matrix? OpenGL uses column-major, so no.
                           GL_FALSE,
                        // Pointer to the first element of the matrix
                           &model[0][0]);
    }
}

void ShaderProgram::setViewMatrix(const glm::mat3 &vp)
{
    // Tell OpenGL to use this shader program for subsequent function calls
    useMe();

    if (m_unifView != -1)
    {
    // Pass a 3x3 matrix into a uniform variable in our shader
                    // Handle to the matrix variable on the GPU
    context->glUniformMatrix3fv(m_unifView,
                    // How many matrices to pass
                       1,
                    // Transpose the matrix? OpenGL uses column-major, so no.
                       GL_FALSE,
                    // Pointer to the first element of the matrix
                       &vp[0][0]);
    }
}

//This function, as its name implies, uses the passed in GL widget
void ShaderProgram::draw(OpenGLContext &f, Drawable &d)
{
    if(d.elemCount() < 0) {
        throw std::invalid_argument(
        "Attempting to draw a Drawable that has not initialized its count variable! Remember to set it to the length of your index array in create()."
        );
    }
    
    useMe();

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

    if (m_attrPos != -1 && d.bindPos())
    {
        context->glEnableVertexAttribArray(m_attrPos);
        context->glVertexAttribPointer(m_attrPos, 3, GL_FLOAT, false, 0, NULL);
    }

    if (m_attrCol != -1 && d.bindCol())
    {
        context->glEnableVertexAttribArray(m_attrCol);
        context->glVertexAttribPointer(m_attrCol, 3, GL_FLOAT, false, 0, NULL);
    }

    // Bind the index buffer and then draw shapes from it.
    // This invokes the shader program, which accesses the vertex buffers.
    d.bindIdx();
    f.glDrawElements(d.drawMode(), d.elemCount(), GL_UNSIGNED_INT, 0);

    if (m_attrPos != -1) context->glDisableVertexAttribArray(m_attrPos);
    if (m_attrCol != -1) context->glDisableVertexAttribArray(m_attrCol);

    f.printGLErrorLog();
}

char* ShaderProgram::textFileRead(const char* fileName) {
    char* text;

    if (fileName != NULL) {
        FILE *file = fopen(fileName, "rt");

        if (file != NULL) {
            fseek(file, 0, SEEK_END);
            int count = ftell(file);
            rewind(file);

            if (count > 0) {
                text = (char*)malloc(sizeof(char) * (count + 1));
                count = fread(text, sizeof(char), count, file);
                text[count] = '\0';	//cap off the string with a terminal symbol, fixed by Cory
            }
            fclose(file);
        }
    }
    return text;
}



QString ShaderProgram::qTextFileRead(const char *fileName)
{
    QString text;
    QFile file(fileName);
    if (file.open(QFile::ReadOnly))
    {
        QTextStream in(&file);
        text = in.readAll();
        text.append('\0');
    }
    return text;
}

void ShaderProgram::printShaderInfoLog(int shader)
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    context->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);

    // should additionally check for OpenGL errors here

    if (infoLogLen > 0)
    {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        context->glGetShaderInfoLog(shader,infoLogLen, &charsWritten, infoLog);
        qDebug() << "ShaderInfoLog:" << "\n" << infoLog << "\n";
        delete [] infoLog;
    }

    // should additionally check for OpenGL errors here
}

void ShaderProgram::printLinkInfoLog(int prog)
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    context->glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLogLen);

    // should additionally check for OpenGL errors here

    if (infoLogLen > 0) {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        context->glGetProgramInfoLog(prog, infoLogLen, &charsWritten, infoLog);
        qDebug() << "LinkInfoLog:" << "\n" << infoLog << "\n";
        delete [] infoLog;
    }
}
