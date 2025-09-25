#include "Ui.h"

#include <stdio.h>
#include <GL/freeglut.h>
#include "Tool.h"
#include "ChannelHandler.h"
#include "MeasurementTable.h"

#include <sys/types.h>
#ifdef __WIN32__
#include <winsock2.h>
#define SEND_FLAGS 0
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#define SEND_FLAGS MSG_DONTWAIT
#endif
#include <strings.h>
#include <iostream>
#include <cstring>

#ifdef __WIN32__
int inet_aton(const char *address, struct in_addr *sock)
{
    sock->s_addr = inet_addr(address);
    return 1;
}
#endif

CUi::CUi(CTool* pTool, char* ip)
{
    m_pTool = pTool;
    m_eActiveOverlay = NoOverlay;
    m_iZoomOut = 1;
    m_iSkip = 1;
    
    initNet(ip);
}

CUi::~CUi(void)
{
}

void CUi::handleKey(char key)
{
    switch(m_eActiveOverlay)
    {
        case NoOverlay:
        {
            switch(key)
            {
                case 27:
                {
                    glutLeaveMainLoop();
                    break;
                }
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                {
                    m_eActiveOverlay = (Overlay_t) (OverlayChannelOne + (key - '1'));
                    break;
                }
                case 'p':
                {
                    m_eActiveOverlay = OverlayParameter;
                    break;
                }
                case ' ':
                {
                    sendPacket('x', 0);
                    break;
                }
                case 'i':
                {
                    if(m_iZoomOut > 1) m_iZoomOut--;
                    break;
                }
                case 'o':
                {
                    if(m_iZoomOut < 10) m_iZoomOut++;
                    break;
                }
                case '+':
                {
                    sendPacket('i', 1 * m_pTool->getChannelHandler()->getParameterIncrement());
                    break;
                }
                case '*':
                {
                    sendPacket('i', 10 * m_pTool->getChannelHandler()->getParameterIncrement());
                    break;
                }
                case '-':
                {
                    sendPacket('i', -1 * m_pTool->getChannelHandler()->getParameterIncrement());
                    break;
                }
                case '_':
                {
                    sendPacket('i', -10 * m_pTool->getChannelHandler()->getParameterIncrement());

                    break;
                }
                case 'r':
                {
                    sendPacket('r', 0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        case OverlayChannelOne:
        case OverlayChannelTwo:
        case OverlayChannelThree:
        case OverlayChannelFour:
        case OverlayChannelFive:
        {
            switch(key)
            {
                case 27:
                {
                    m_eActiveOverlay = NoOverlay;
                    break;
                }
                default:
                {
                    sendPacket('c', (m_eActiveOverlay - OverlayChannelOne) << 8 | key);
                    m_eActiveOverlay = NoOverlay;
                    break;
                }
            }
            break;
        }
        case OverlayParameter:
        {
            switch(key)
            {
                case 27:
                {
                    m_eActiveOverlay = NoOverlay;
                    break;
                }
                default:
                {
                    sendPacket('p', key);
                    m_eActiveOverlay = NoOverlay;
                    break;
                }
            }
            break;
        }
    }
    glutPostRedisplay();
}

void CUi::checkKeyRepeat(void)
{
    if(m_cKeyRepeat)
    {
        if(m_iKeyRepeatTimer == 0)
        {
            m_iKeyRepeatTimer = 2;
            handleKey(m_cKeyRepeat);
        }
        else
        {
            m_iKeyRepeatTimer--;
        }
    }
}

typedef struct {int8_t a;int8_t b;int8_t c;int8_t d;} SInput;
static SInput inVec = {0};
void CUi::keyDown(char key)
{
    if(m_eActiveOverlay == NoOverlay)
    {
        switch(key)
        {
            case 'w': inVec.a = 127; break;
            case 's': inVec.a = -127; break;
            case 'a': inVec.b = 127; break;
            case 'd': inVec.b = -127; break;
        }
        sendPacket('a', *((int*)&inVec));
    }
    m_cKeyRepeat = key;
    m_iKeyRepeatTimer = 30;
    handleKey(m_cKeyRepeat);
}

void CUi::keyUp(char key)
{
    if(m_eActiveOverlay == NoOverlay)
    {
        switch(key)
        {
            case 'w': inVec.a = 0; break;
            case 's': inVec.a = 0; break;
            case 'a': inVec.b = 0; break;
            case 'd': inVec.b = 0; break;
        }
        sendPacket('a', *((int*)&inVec));
    }
    m_cKeyRepeat = 0;
}

void CUi::mouseButton(int button, int state)
{
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
    }
    if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
    }
}

void CUi::mouseMotion(int x, int y)
{
    m_iMouseX = x;
    m_iMouseY = y;
}

void CUi::draw(int width, int height)
{
    switch(m_eActiveOverlay)
    {
        case NoOverlay:
        {
            break;
        }
        case OverlayChannelOne:
        case OverlayChannelTwo:
        case OverlayChannelThree:
        case OverlayChannelFour:
        case OverlayChannelFive:
        {
            glColor3f(0.0f, 0.0f, 0.0f);
            glRectf(GLfloat(width / 10 + 1), GLfloat(height / 10 + 1), GLfloat(9 * width / 10), GLfloat(9 * height / 10 - 1));

            sprintf(m_sBuffer, "Messwertauswahl Kanal %d", m_eActiveOverlay - OverlayChannelOne + 1);
            glColor3f(1.0f, 1.0f, 1.0f);
            glRasterPos2f(GLfloat(width / 10 + 1 + 40), GLfloat(height / 10 + 1 + 14 + 40));
            glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)m_sBuffer);

            for(int i = 0; measurementTable[i].id; ++i)
            {
                sprintf(m_sBuffer, "%c: %s", measurementTable[i].id, measurementTable[i].name.c_str());
                glColor3f(1.0f, 1.0f, 1.0f);
                glRasterPos2f(GLfloat(width / 10 + 1 + 40), GLfloat(height / 10 + 1 + 14 + 40 + 24 * (i + 2)));
                glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)m_sBuffer);
            }

            break;
        }
        case OverlayParameter:
        {
            glColor3f(0.0f, 0.0f, 0.0f);
            glRectf(GLfloat(width / 10 + 1), GLfloat(height / 10 + 1), GLfloat(9 * width / 10), GLfloat(9 * height / 10 - 1));

            sprintf(m_sBuffer, "Parameterauswahl");
            glColor3f(1.0f, 1.0f, 1.0f);
            glRasterPos2f(GLfloat(width / 10 + 1 + 40), GLfloat(height / 10 + 1 + 14 + 40));
            glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)m_sBuffer);

            for(int i = 0; parameterTable[i].id; ++i)
            {
                sprintf(m_sBuffer, "%c: %s", parameterTable[i].id, parameterTable[i].name.c_str());
                glColor3f(1.0f, 1.0f, 1.0f);
                glRasterPos2f(GLfloat(width / 10 + 1 + 40), GLfloat(height / 10 + 1 + 14 + 40 + 24 * (i + 2)));
                glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)m_sBuffer);
            }

            break;
        }
    }
}

void CUi::initNet(char* ip)
{
    struct sockaddr_in servaddr;

    #ifdef _WIN32
    WSADATA wsaData;
    int wsa = WSAStartup(MAKEWORD(2, 2), &wsaData);
    std::cout << "wsa start (ui): " << wsa << std::endl;
    #endif

    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    std::cout << "udpfd (ui): " << udpfd << std::endl;

    #ifdef _WIN32
    u_long mode = 0;
    int ioctl = ioctlsocket(udpfd, FIONBIO, &mode);
    std::cout << "ioctl (ui): " << ioctl << std::endl;
    #endif

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(23456);
    inet_aton(ip, (in_addr*)&servaddr.sin_addr.s_addr);

    int cn = connect(udpfd, (sockaddr*)&servaddr, sizeof(servaddr));
    std::cout << "cn (ui): " << cn << std::endl;
}

void CUi::sendPacket(uint8_t command, int32_t data)
{
    struct
    {
       uint8_t type;
       int32_t data;
    } dataout;

    dataout.type = command;
    dataout.data = data;

    send(udpfd, (char *)&dataout, sizeof(dataout), SEND_FLAGS);
}

void CUi::sendPacket(uint8_t command, float data)
{
    struct
    {
       uint8_t type;
       float data;
    } dataout;

    dataout.type = command;
    dataout.data = data;

    send(udpfd, (char *)&dataout, sizeof(dataout), SEND_FLAGS);
}

int CUi::getZoomOut()
{
    return m_iZoomOut;
}
