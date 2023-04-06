#include "IPCtrl.h"

IPCtrl::IPCtrl(QWidget *parent) : QFrame(parent)
{
    setFrameShape( QFrame::StyledPanel );

    // BG modified
    setFrameShadow( QFrame::Plain ); 
    // BG modified end

    QHBoxLayout* pLayout = new QHBoxLayout( this );
    setLayout( pLayout );
    pLayout->setContentsMargins( 0, 0, 0, 0 );
    pLayout->setSpacing( 0 );

    for ( int i = 0; i != QTUTL_IP_SIZE; ++i )
    {
        if ( i != 0 )
        {
            QLabel* pDot = new QLabel( ".", this );
            pDot->setStyleSheet( "background: white" );
            pLayout->addWidget( pDot );
            pLayout->setStretch( pLayout->count(), 0 );
        }

        m_pLineEdit[i] = new QLineEdit( this );
        QLineEdit* pEdit = m_pLineEdit[i];
        pEdit->installEventFilter( this );

        pLayout->addWidget( pEdit );
        pLayout->setStretch( pLayout->count(), 1 );

        pEdit->setFrame( false );
        pEdit->setAlignment( Qt::AlignCenter );

        QFont font = pEdit->font();
        font.setStyleHint( QFont::Monospace );
        font.setFixedPitch( true );
        pEdit->setFont( font );

        QRegExp rx ( "^(0|[1-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))$" );
        QValidator *validator = new QRegExpValidator(rx, pEdit);
        pEdit->setValidator( validator );

    }

    // BG modified
    setMaximumWidth( 30 * QTUTL_IP_SIZE +10);
    // BG modified end

    connect( this, SIGNAL(signalTextChanged(QLineEdit*)),
             this, SLOT(slotTextChanged(QLineEdit*)),
             Qt::QueuedConnection );
}

IPCtrl::~IPCtrl()
{

}

// BG
void IPCtrl::setValue(QString ip) {
    QStringList subStrings = ip.split('.');

    for ( int i = 0; i != QTUTL_IP_SIZE; ++i )
        if(subStrings.size()>=i+1 && subStrings[i].size()>=1 && subStrings[i].size() <=3)
            m_pLineEdit[i]->setText(subStrings[i]);
        else
            m_pLineEdit[i]->setText("0");
}

// BG
QString IPCtrl::getValue() {
    return QString( m_pLineEdit[0]->text() + '.' + m_pLineEdit[1]->text() + '.' + m_pLineEdit[2]->text() + '.' + m_pLineEdit[3]->text() );
}

void IPCtrl::slotTextChanged( QLineEdit* pEdit )
{
    for ( unsigned int i = 0; i != QTUTL_IP_SIZE; ++i )
    {
        if ( pEdit == m_pLineEdit[i] )
        {
            if ( ( pEdit->text().size() == MAX_DIGITS &&  pEdit->text().size() == pEdit->cursorPosition() ) || ( pEdit->text() == "0") )
            {
                // auto-move to next item
                if ( i+1 != QTUTL_IP_SIZE )
                {
                   m_pLineEdit[i+1]->setFocus();
                   m_pLineEdit[i+1]->selectAll();
                }
            }
        }
    }
}

bool IPCtrl::eventFilter(QObject *obj, QEvent *event)
{
    bool bRes = QFrame::eventFilter(obj, event);

    if ( event->type() == QEvent::KeyPress )
    {
        QKeyEvent* pEvent = dynamic_cast<QKeyEvent*>( event );
        if ( pEvent )
        {
            for ( unsigned int i = 0; i != QTUTL_IP_SIZE; ++i )
            {
                QLineEdit* pEdit = m_pLineEdit[i];
                if ( pEdit == obj )
                {
                    switch ( pEvent->key() )
                    {
                    case Qt::Key_Left:
                        if ( pEdit->cursorPosition() == 0 )
                        {
                            // user wants to move to previous item
                            MovePrevLineEdit(i);
                        }
                        break;

                    case Qt::Key_Right:
                        if ( pEdit->text().isEmpty() || (pEdit->text().size() == pEdit->cursorPosition()) )
                        {
                            // user wants to move to next item
                            MoveNextLineEdit(i);
                        }
                        break;

                    case Qt::Key_0:
                        if ( pEdit->text().isEmpty() || pEdit->text() == "0" )
                        {
                            pEdit->setText("0");
                            // user wants to move to next item
                            MoveNextLineEdit(i);
                        }
                        emit signalTextChanged( pEdit );
                        break;

                    case Qt::Key_Backspace:
                        if ( pEdit->text().isEmpty() || pEdit->cursorPosition() == 0)
                        {
                            // user wants to move to previous item
                            MovePrevLineEdit(i);
                        }
                        break;

                    case Qt::Key_Comma:
                    case Qt::Key_Period:
                        MoveNextLineEdit(i);
                        break;

                    default:
                        emit signalTextChanged( pEdit );
                        break;

                    }
                }
            }
        }
    }

    return bRes;
}

void IPCtrl::MoveNextLineEdit(int i)
{
    if ( i+1 != QTUTL_IP_SIZE )
    {
        m_pLineEdit[i+1]->setFocus();
        m_pLineEdit[i+1]->setCursorPosition( 0 );
        m_pLineEdit[i+1]->selectAll();
    }
}

void IPCtrl::MovePrevLineEdit(int i)
{
    if ( i != 0 )
    {
        m_pLineEdit[i-1]->setFocus();
        m_pLineEdit[i-1]->setCursorPosition( m_pLineEdit[i-1]->text().size() );
        //m_pLineEdit[i-1]->selectAll();
    }
}