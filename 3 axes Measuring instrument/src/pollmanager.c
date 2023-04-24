//
// Created by hong on 2021-07-16.
//

#include "../include/pollmanager.h"
#include "../include/global.h"
#include "../include/netinc.h"



typedef struct pollfd pollfd_t;

pollfd_t    poll_array[POLL_MAX_COUNT];                               // poll �迭
net_obj_t  *poll_objs[POLL_MAX_COUNT];                                // poll �� �ش�Ǵ� object

//--------------------------------------------------------------------
// ����: poll �޴����� �ʱ�ȭ
//--------------------------------------------------------------------
void  poll_init( void)
{
    int   ndx;
    // poll array �� �ʱ�ȭ �Ѵ�.
    // pollfd ����ü�� ��� fd �� -1 �� �ʱ�ȭ �Ѵ�.
    // fd �� -1 �̸� ���������ڰ� ���õǾ����� �ʴٴ� ���̴�.

    for ( ndx = 0; ndx < POLL_MAX_COUNT; ndx++)
    {
        poll_array[ndx].fd  = -1;
        poll_objs[ndx]      = NULL;
    }
}

//--------------------------------------------------------------------
// ����: poll �޴������� ��� �ִ� �ε��� ��ȣ�� ���Ѵ�.
// ��ȯ: �迭 �� ��ϵ��� ���� �ε��� ��ȣ
//--------------------------------------------------------------------
int poll_blank_ndx( void)
{
    int poll_ndx;
    int ndx;

    // poll array ���� ��ũ���Ͱ� -1 �� ��������
    // poll �˻����� ������� �ʴ� �������̴�.
    // ��� �ִ� �迭�� �ε����� �˻��Ѵ�.

    poll_ndx  = -1;
    for ( ndx = 0; ndx < POLL_MAX_COUNT; ndx++)
    {
        if ( -1 == poll_array[ndx].fd)
        {
            poll_ndx  = ndx;                                                // ��� �ִ�  array �� ã��
            break;
        }
    }
    return poll_ndx;
}

//--------------------------------------------------------------------
// ����: poll �޴����� net_obj �� ���
//--------------------------------------------------------------------
int poll_register( net_obj_t *_obj)
{
    int poll_ndx;

    // ��� �ִ� poll array �� ã�� ������ ����Ѵ�.
    // poll array �� ����� �� �ִ� �ε����� ���� ���
    // poll_ndx �� 0 �̻��� ���̵ȴ�.

    poll_ndx  = poll_blank_ndx();

    if ( 0 <= poll_ndx)                                                 // ����� �� �ִ� poll array �� ã����
    {
        _obj->poll_ndx                = poll_ndx;                         // socket ������ �� ������ ���
        poll_array[poll_ndx].fd       = _obj->fd;                         // poll �� ��ũ���͸� ����Ѵ�.
        poll_array[poll_ndx].events   = POLLIN | POLLERR | POLLHUP;       // POLLOUT �� poll_change_event() ���� ���� ���
        poll_array[poll_ndx].revents  = 0;
        poll_objs[poll_ndx]           = _obj;
    }
    return poll_ndx;
}

//--------------------------------------------------------------------
// ����: poll �޴����� net_obj �� �Ҹ�
//--------------------------------------------------------------------
void poll_unregister( net_obj_t *_obj)
{
    poll_array[_obj->poll_ndx].fd = -1;                                 // poll array �� �迭�� blank �� ����
    poll_objs[_obj->poll_ndx]     = NULL;                               // poll array �� �ش�Ǵ� obj ������ ����
}

//--------------------------------------------------------------------
// ����: poll �޴����� �ִ� � ��ü ����
// ��ȯ: �ִ� ��ü ����
//--------------------------------------------------------------------
int poll_count( void)
{
    return  POLL_MAX_COUNT;
}

//--------------------------------------------------------------------
// ����: poll �޴����� ��ϵ� net_obj �� ��ȯ
// �μ�: _ndx   : �ε��� ��ȣ
// ��ȯ: net_obj_t
//--------------------------------------------------------------------
net_obj_t *poll_obj( int _ndx)
{
    return  poll_objs[_ndx];
}

//--------------------------------------------------------------------
// ����: poll �޴����� ��ϵ� ��ũ���Ϳ��� �ε��� ��ȣ�� �ش��ϴ�
//       ��ũ���͸� ��ȯ�Ѵ�.
// �μ�: _ndx   : �ε��� ��ȣ
// ��ȯ: ��ũ����
//--------------------------------------------------------------------
int poll_get_fd( int _ndx)
{
    return  poll_array[_ndx].fd;
}

//--------------------------------------------------------------------
// ����: poll �޴����� ��ϵ� ��ũ���Ϳ��� �ε��� ��ȣ�� ����
//       ��ũ���͸� �����Ѵ�.
// �μ�: _ndx   : �ε��� ��ȣ
//       _fd    : ��ũ����
//--------------------------------------------------------------------
void poll_set_fd( int _ndx, int _fd)
{
    poll_array[_ndx].fd = _fd;
}

//--------------------------------------------------------------------
// ����: ������ ���Ͽ� ���� Ȯ���ϴ� �̺�Ʈ ������ �ٽ� �����Ѵ�.
// �μ�: _fd    : ������ ��ũ����
//       _event : �����Ϸ��� �̺�Ʈ ����
// ��ȯ: 0                  : ���� ����
//       POLL_NO_DESCRIPTOR : �μ��� ���� ��ũ���Ͱ� poll �޴�����
//                            ��ϵǾ� ���� ����
//--------------------------------------------------------------------
int poll_change_event( int _fd, int _event)
{
    int   ndx;

    for( ndx = 0; ndx < POLL_MAX_COUNT; ndx++ )
    {
        if ( _fd == poll_array[ndx].fd)
        {
            poll_array[ndx].events  = _event;
            return 0;
        }
    }
    return POLL_NO_DESCRIPTOR;
}

//--------------------------------------------------------------------
// ����: poll ������ ����
// ��ȯ: POLL_BIG_ERR   - ġ������ ������ �߻�
//       POLL_TIME_OUT  - ������ �ð����� �̺�Ʈ�� �߻����� �ʾ���
//       POLL_EVENTED   - Ư�� �̺�Ʈ�� �߻�����
//--------------------------------------------------------------------
int poll_loop( int _time_out)
{
    int     poll_state;
    int     ndx;

    poll_state = poll( (struct pollfd *)&poll_array, POLL_MAX_COUNT, _time_out);
    if ( 0 > poll_state)
    {                                                                   // ġ������ ������ �߻��ߴ�.
        return POLL_BIG_ERR;
    }
    if ( 0 == poll_state)                                               // Ÿ�Ӿƿ��� �ɷ����Ƿ�
    {                                                                   // �ٽ� ���¸� Ȯ���Ѵ�.
        return POLL_TIME_OUT;
    }
    for( ndx = 0; ndx < POLL_MAX_COUNT; ndx++ )
    {
        if( poll_array[ndx].revents & POLLIN )                            // ���κ��� �ڷᰡ ���ŵǾ���
        {
            if ( NULL != poll_objs[ndx])
            {
                poll_objs[ndx]->on_poll_in( poll_objs[ndx]);
            }
        }
        if( poll_array[ndx].revents & POLLOUT )                           // ���� ���°� block �� �ƴ�
        {
            if ( NULL != poll_objs[ndx])
            {
                poll_objs[ndx]->on_poll_out( poll_objs[ndx]);
            }
        }
        if( poll_array[ndx].revents & POLLERR )                           // ������ �߻�
        {
            if ( NULL != poll_objs[ndx])
            {
                poll_objs[ndx]->on_poll_err( poll_objs[ndx]);
            }
        }
        if( poll_array[ndx].revents & POLLHUP )                           // ������ ���� ���ٸ�
        {
            if ( NULL != poll_objs[ndx])
            {
                poll_objs[ndx]->on_poll_hup( poll_objs[ndx]);
            }
        }
    }
    return  POLL_EVENTED;
}
