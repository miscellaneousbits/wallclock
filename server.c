#include "main.h"
#include "src/shared/mgmt.h"
#include "src/advertising.h"

static char* name = "BLE";

const uint16_t UUID_GAP = 0x1800;
const uint16_t UUID_GATT = 0x1801;
const char* UUID_I2C_READ = "000A0001-1000-8000-0080-5F9B34FB0000";
const char* UUID_I2C_READ_DATA = "000A0002-0010-0080-0000-805F9B34FB00";
const char* UUID_I2C_WRITE = "000A0003-0010-0080-0000-805F9B34FB00";
const char* UUID_I2C_WRITE_DATA = "000A0004-0010-0080-0000-805F9B34FB00";

#define ATT_CID 4

static char* device_name = "WALLCLOCK";

static void att_disconnect_cb(int err, void* user_data)
{
    mainloop_quit();
}

static void gatt_service_changed_cb(struct gatt_db_attribute* attrib, uint32_t id, uint16_t offset,
    uint8_t opcode, struct bt_att* att, void* user_data)
{
    gatt_db_attribute_read_result(attrib, id, 0, NULL, 0);
}

static void gatt_svc_chngd_ccc_read_cb(struct gatt_db_attribute* attrib, uint32_t id,
    uint16_t offset, uint8_t opcode, struct bt_att* att, void* user_data)
{
    struct server* server = user_data;
    uint8_t value[2];

    value[0] = server->svc_chngd_enabled ? 0x02 : 0x00;
    value[1] = 0x00;

    gatt_db_attribute_read_result(attrib, id, 0, value, sizeof(value));
}

static void gatt_svc_chngd_ccc_write_cb(struct gatt_db_attribute* attrib, uint32_t id,
    uint16_t offset, const uint8_t* value, size_t len, uint8_t opcode, struct bt_att* att,
    void* user_data)
{
    struct server* server = user_data;
    uint8_t ecode = 0;

    if (!value || len != 2)
    {
        ecode = BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN;
        goto done;
    }

    if (offset)
    {
        ecode = BT_ATT_ERROR_INVALID_OFFSET;
        goto done;
    }

    if (value[0] == 0x00)
        server->svc_chngd_enabled = false;
    else if (value[0] == 0x02)
        server->svc_chngd_enabled = true;
    else
        ecode = 0x80;

done:
    gatt_db_attribute_write_result(attrib, id, ecode);
}

static void clock_read_data_ccc_read_cb(struct gatt_db_attribute* attrib, uint32_t id,
    uint16_t offset, uint8_t opcode, struct bt_att* att, void* user_data)
{
    struct server* server = user_data;
    uint8_t value[2];

    value[0] = server->clock_read_data_notify ? 0x01 : 0x00;
    value[1] = 0x00;

    gatt_db_attribute_read_result(attrib, id, 0, value, 2);
}

bool clock_read_data_cb(void* user_data)
{
    user_data_t* u_data = (user_data_t*)user_data;
    struct server* server = u_data->server;

    bt_gatt_server_send_notification(
        server->gatt, server->clock_read_data_handle, u_data->buffer, u_data->len);

    return true;
}

static void clock_read_data_ccc_write_cb(struct gatt_db_attribute* attrib, uint32_t id,
    uint16_t offset, const uint8_t* value, size_t len, uint8_t opcode, struct bt_att* att,
    void* user_data)
{
    struct server* server = user_data;
    uint8_t ecode = 0;

    if (!value || len != 2)
    {
        ecode = BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN;
        goto done;
    }

    if (offset)
    {
        ecode = BT_ATT_ERROR_INVALID_OFFSET;
        goto done;
    }

    if (value[0] == 0x00)
        server->clock_read_data_notify = false;
    else if (value[0] == 0x01)
        server->clock_read_data_notify = true;
    else
        ecode = 0x80;

done:
    gatt_db_attribute_write_result(attrib, id, ecode);
}

static void clock_write_data_write_cb(struct gatt_db_attribute* attrib, uint32_t id,
    uint16_t offset, const uint8_t* value, size_t len, uint8_t opcode, struct bt_att* att,
    void* user_data)
{
    struct server* server = user_data;
    uint8_t ecode = 0;

    if (!server || !value || len == 0 || len > 100)
    {
        ecode = BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN;
        goto done;
    }

    if (offset)
    {
        ecode = BT_ATT_ERROR_INVALID_OFFSET;
        goto done;
    }

    clock_command(server, (const uint8_t*)value, len);

done:
    gatt_db_attribute_write_result(attrib, id, ecode);
}

static void confirm_write(struct gatt_db_attribute* attr, int err, void* user_data)
{
    if (!err)
        return;

    fprintf(stderr, LOG_ERR_STR "%s: Error caching attribute %p - err: %d\n", name, attr, err);
    fflush(stderr);
    exit(1);
}

static int populate_gap_service(struct server* server)
{
    bt_uuid_t uuid;
    struct gatt_db_attribute *service, *tmp;
    uint16_t appearance;
    uint16_t preference[4];

    /* Add the GAP service */
    bt_uuid16_create(&uuid, UUID_GAP);
    service = gatt_db_add_service(server->db, &uuid, true, 7);
    if (service == NULL)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to add GAP service\n", name);
        fflush(stderr);
        goto fail;
    }

    /*
        Device Name characteristic. Make the value dynamically read and
        written via callbacks.
    */
    bt_uuid16_create(&uuid, GATT_CHARAC_DEVICE_NAME);
    tmp = gatt_db_service_add_characteristic(
        service, &uuid, BT_ATT_PERM_READ, BT_GATT_CHRC_PROP_READ, NULL, NULL, server);
    if (tmp == NULL)
    {
        fprintf(
            stderr, LOG_ERR_STR "%s: Failed to add GAP service device name characteristic\n", name);
        fflush(stderr);
        goto fail;
    }

    /*
        Write the device name value to the database, since we're not using a
        callback.
    */
    if (!gatt_db_attribute_write(tmp, 0, (void*)server->device_name, server->name_len,
            BT_ATT_OP_WRITE_REQ, NULL, confirm_write, NULL))
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to write GAP service device name characteristic\n",
            name);
        fflush(stderr);
        goto fail;
    }
    /*
        Appearance characteristic. Reads and writes should obtain the value
        from the database.
    */
    bt_uuid16_create(&uuid, GATT_CHARAC_APPEARANCE);
    tmp = gatt_db_service_add_characteristic(
        service, &uuid, BT_ATT_PERM_READ, BT_GATT_CHRC_PROP_READ, NULL, NULL, server);
    if (tmp == NULL)
    {
        fprintf(
            stderr, LOG_ERR_STR "%s: Failed to add GAP service appearance characteristic\n", name);
        fflush(stderr);
        goto fail;
    }

    /*
        Write the appearance value to the database, since we're not using a
        callback.
    */
    put_le16(0, &appearance);
    if (!gatt_db_attribute_write(tmp, 0, (void*)&appearance, sizeof(appearance),
            BT_ATT_OP_WRITE_REQ, NULL, confirm_write, NULL))
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to write GAP service appearance characteristic\n",
            name);
        fflush(stderr);
        goto fail;
    }

    /*
        peripheral preferred Connection characteristic. Reads should obtain the value
        from the database.
    */
    bt_uuid16_create(&uuid, GATT_CHARAC_PERIPHERAL_PREF_CONN);
    tmp = gatt_db_service_add_characteristic(
        service, &uuid, BT_ATT_PERM_READ, BT_GATT_CHRC_PROP_READ, NULL, NULL, server);
    if (tmp == NULL)
    {
        fprintf(stderr,
            LOG_ERR_STR "%s: Failed to add GAP service prefered connection characteristic\n", name);
        fflush(stderr);
        goto fail;
    }

    /*
        Write the preference value to the database, since we're not using a
        callback.
    */
    put_le16(6, &preference[0]);
    put_le16(8, &preference[1]);
    put_le16(0, &preference[2]);
    put_le16(0xfa, &preference[3]);
    if (!gatt_db_attribute_write(tmp, 0, (void*)preference, sizeof(preference), BT_ATT_OP_WRITE_REQ,
            NULL, confirm_write, NULL))
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to write GAP service appearance characteristic\n",
            name);
        fflush(stderr);
        goto fail;
    }

    gatt_db_service_set_active(service, true);
    return 0;

fail:
    return -1;
}

static int populate_gatt_service(struct server* server)
{
    bt_uuid_t uuid;
    struct gatt_db_attribute *service, *svc_chngd;

    /* Add the GATT service */
    bt_uuid16_create(&uuid, UUID_GATT);
    service = gatt_db_add_service(server->db, &uuid, true, 4);
    if (service == NULL)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to add GATT service\n", name);
        fflush(stderr);
        goto fail;
    }

    bt_uuid16_create(&uuid, GATT_CHARAC_SERVICE_CHANGED);
    svc_chngd = gatt_db_service_add_characteristic(service, &uuid, BT_ATT_PERM_READ,
        BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_INDICATE, gatt_service_changed_cb, NULL, server);
    if (svc_chngd == NULL)
    {
        fprintf(
            stderr, LOG_ERR_STR "%s: Failed to add GATT service changed characteristic\n", name);
        fflush(stderr);
        goto fail;
    }
    server->gatt_svc_chngd_handle = gatt_db_attribute_get_handle(svc_chngd);

    bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);
    if (gatt_db_service_add_descriptor(service, &uuid, BT_ATT_PERM_READ | BT_ATT_PERM_WRITE,
            gatt_svc_chngd_ccc_read_cb, gatt_svc_chngd_ccc_write_cb, server) == NULL)
    {
        fprintf(stderr,
            LOG_ERR_STR "%s: Failed to add GATT service characteristic config descriptor\n", name);
        fflush(stderr);
        goto fail;
    }

    gatt_db_service_set_active(service, true);
    return 0;
fail:
    return -1;
}

static int populate_clock_read_service(struct server* server)
{
    bt_uuid_t uuid;
    struct gatt_db_attribute *service, *clock_read_data;

    /* Add I2C_Read Service */
    bt_string_to_uuid(&uuid, UUID_I2C_READ);
    service = gatt_db_add_service(server->db, &uuid, true, 4);
    if (service == NULL)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to add I2C_Read service\n", name);
        fflush(stderr);
        goto fail;
    }
    /* I2c_Read_data Characteristic */
    bt_string_to_uuid(&uuid, UUID_I2C_READ_DATA);
    clock_read_data = gatt_db_service_add_characteristic(service, &uuid,
        BT_ATT_PERM_READ | BT_ATT_PERM_WRITE, BT_GATT_CHRC_PROP_NOTIFY, NULL, NULL, NULL);
    if (clock_read_data == NULL)
    {
        fprintf(stderr,
            LOG_ERR_STR "%s: Failed to add I2C_Read service I2C_Read_data characteristic\n", name);
        fflush(stderr);
        goto fail;
    }
    server->clock_read_data_handle = gatt_db_attribute_get_handle(clock_read_data);

    bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);
    if (gatt_db_service_add_descriptor(service, &uuid, BT_ATT_PERM_READ | BT_ATT_PERM_WRITE,
            clock_read_data_ccc_read_cb, clock_read_data_ccc_write_cb, server) == NULL)
    {
        fprintf(stderr,
            LOG_ERR_STR "%s: Failed to add I2C_Read service characteristic config descriptor\n",
            name);
        fflush(stderr);
        goto fail;
    }

    gatt_db_service_set_active(service, true);
    return 0;
fail:
    return -1;
}

static int populate_clock_write_service(struct server* server)
{
    bt_uuid_t uuid;
    struct gatt_db_attribute *service, *clock_write_data;

    /* Add I2C_Read Service */
    bt_string_to_uuid(&uuid, UUID_I2C_WRITE);
    service = gatt_db_add_service(server->db, &uuid, true, 3);
    if (service == NULL)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to add I2C_Write service\n", name);
        fflush(stderr);
        goto fail;
    }
    /* I2c_Write_data Characteristic */
    bt_string_to_uuid(&uuid, UUID_I2C_WRITE_DATA);
    clock_write_data = gatt_db_service_add_characteristic(service, &uuid, BT_ATT_PERM_WRITE,
        BT_GATT_CHRC_PROP_WRITE_WITHOUT_RESP, NULL, clock_write_data_write_cb, server);
    if (clock_write_data == NULL)
    {
        fprintf(stderr,
            LOG_ERR_STR "%s: Failed to add I2C_Write service I2C_Write_data characteristic\n",
            name);
        fflush(stderr);
        goto fail;
    }
    server->clock_write_data_handle = gatt_db_attribute_get_handle(clock_write_data);

    gatt_db_service_set_active(service, true);
    return 0;
fail:
    return -1;
}

static int populate_db(struct server* server)
{
    if (populate_gap_service(server) < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to populate GAP service\n", name);
        fflush(stderr);
        return -1;
    }
    if (populate_gatt_service(server) < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to populate GATT service\n", name);
        fflush(stderr);
        return -1;
    }
    if (populate_clock_read_service(server) < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to populate I2C_Read service\n", name);
        fflush(stderr);
        return -1;
    }
    if (populate_clock_write_service(server) < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to populate I2C_Write service\n", name);
        fflush(stderr);
        return -1;
    }
    return 0;
}

static struct server* server_create(int fd, uint16_t mtu)
{
    struct server* server;

    server = new0(struct server, 1);
    if (!server)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to allocate memory for server\n", name);
        fflush(stderr);
        return NULL;
    }

    server->att = bt_att_new(fd, false);
    if (!server->att)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to initialze ATT transport layer\n", name);
        fflush(stderr);
        goto fail;
    }

    if (!bt_att_set_close_on_unref(server->att, true))
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to set up ATT transport layer\n", name);
        fflush(stderr);
        goto fail;
    }

    if (!bt_att_register_disconnect(server->att, att_disconnect_cb, NULL, NULL))
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to set ATT disconnect handler\n", name);
        fflush(stderr);
        goto fail;
    }

    server->name_len = strlen(device_name) + 1;
    server->device_name = device_name;

    server->fd = fd;
    server->db = gatt_db_new();
    if (!server->db)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to create GATT database\n", name);
        fflush(stderr);
        goto fail;
    }

    server->gatt = bt_gatt_server_new(server->db, server->att, mtu, 0);
    if (!server->gatt)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to create GATT server\n", name);
        fflush(stderr);
        goto fail;
    }

    /* bt_gatt_server already holds a reference */
    if (populate_db(server) < 0)
        goto fail;

    return server;

fail:
    gatt_db_unref(server->db);
    free(server->device_name);
    bt_att_unref(server->att);
    free(server);

    return NULL;
}

static void server_destroy(struct server* server)
{
    bt_gatt_server_unref(server->gatt);
    gatt_db_unref(server->db);
}

static int l2cap_le_att_listen_and_accept(void)
{
    int sk, nsk = -1;

    sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if (sk < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to create L2CAP socket\n", name);
        fflush(stderr);
        return -1;
    }

    /* Set up source address */
    struct sockaddr_l2 srcaddr, addr;
    memset(&srcaddr, 0, sizeof(srcaddr));
    srcaddr.l2_family = AF_BLUETOOTH;
    srcaddr.l2_cid = htobs(ATT_CID);
    srcaddr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
    bacpy(&srcaddr.l2_bdaddr, BDADDR_ANY);

    if (bind(sk, (struct sockaddr*)&srcaddr, sizeof(srcaddr)) < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to bind L2CAP socket\n", name);
        fflush(stderr);
        goto fail;
    }

    /* Set the security level */
    struct bt_security btsec;
    memset(&btsec, 0, sizeof(btsec));
    btsec.level = BT_SECURITY_LOW;
    if (setsockopt(sk, SOL_BLUETOOTH, BT_SECURITY, &btsec, sizeof(btsec)) < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to set L2CAP security level\n", name);
        fflush(stderr);
        goto fail;
    }

    if (listen(sk, 0) < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Listening on socket failed\n", name);
        fflush(stderr);
        goto fail;
    }

    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(sk, &readset);
    struct timeval timeout;
    timeout.tv_sec = 90;
    timeout.tv_usec = 0;
    int rc = select(sk + 1, &readset, NULL, NULL, &timeout);
    if (rc < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Select failed\n", name);
        fflush(stderr);
        goto fail;
    }
    if (rc == 0)
    {
        goto fail;  // timeout
    }

    if (FD_ISSET(sk, &readset))
    {
        memset(&addr, 0, sizeof(addr));
        socklen_t optlen = sizeof(addr);
        nsk = accept(sk, (struct sockaddr*)&addr, &optlen);
        if (nsk < 0)
        {
            fprintf(stderr, LOG_ERR_STR "%s: Accept failed\n", name);
            fflush(stderr);
            goto fail;
        }
    }
fail:
    close(sk);
    return nsk;
}

static int gatt_service(int fd)
{
    mainloop_init();

    struct server* server = server_create(fd, 0);
    if (!server)
    {
        close(fd);
        fprintf(stderr, LOG_ERR_STR "%s: Failed to create server\n", name);
        fflush(stderr);
        return EXIT_FAILURE;
    }

    mainloop_run();

    server_destroy(server);

    return EXIT_SUCCESS;
}

static int restart_advert(void)
{
    int rc = -1;
    int dev_id = hci_get_route(NULL);
    int dd = hci_open_dev(dev_id);
    if (dd < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Device open failed\n", name);
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    /* Setup filter */
    struct hci_filter flt;
    hci_filter_clear(&flt);
    hci_filter_set_ptype(HCI_EVENT_PKT, &flt);
    hci_filter_all_events(&flt);
    if (setsockopt(dd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: HCI filter setup failed\n", name);
        fflush(stderr);
        goto fail;
    }

    static uint8_t cmd1[32] = {0x1E, 0x02, 0x01, 0x1A, 0x1A, 0xFF, 0x4C, 0x00, 0x02, 0x15, 0xE2,
        0x0A, 0x39, 0xF4, 0x73, 0xF5, 0x4B, 0xC4, 0xA1, 0x2F, 0x17, 0xD1, 0xAD, 0x07, 0xA9, 0x61,
        0x00, 0x00, 0x00, 0x00, 0xC8, 0x00};
    static uint8_t cmd2[32] = {0x0c, 0x0a, 0x09, 'W', 'a', 'l', 'l', 'c', 'l', 'o', 'c', 'k', 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00};

    if (hci_send_cmd(dd, 8, 8, sizeof(cmd1), cmd1) < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Send failed, %s\n", name, strerror(errno));
        fflush(stderr);
        goto fail;
    }
    unsigned char buf[HCI_MAX_EVENT_SIZE];
    int len = read(dd, buf, sizeof(buf));
    if (len < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Read failed, %s\n", name, strerror(errno));
        fflush(stderr);
        goto fail;
    }
    if (hci_send_cmd(dd, 8, 9, sizeof(cmd2), cmd2) < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Send failed, %s\n", name, strerror(errno));
        fflush(stderr);
        goto fail;
    }
    len = read(dd, buf, sizeof(buf));
    if (len < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Read failed, %s\n", name, strerror(errno));
        fflush(stderr);
        goto fail;
    }

    struct hci_request rq;
    le_set_advertising_parameters_cp adv_params_cp;
    uint8_t status;

    memset(&adv_params_cp, 0, sizeof(adv_params_cp));
    adv_params_cp.min_interval = htobs(0x0800);
    adv_params_cp.max_interval = htobs(0x0800);
    adv_params_cp.chan_map = 7;
    adv_params_cp.advtype = 0;

    memset(&rq, 0, sizeof(rq));
    rq.ogf = OGF_LE_CTL;
    rq.ocf = OCF_LE_SET_ADVERTISING_PARAMETERS;
    rq.cparam = &adv_params_cp;
    rq.clen = LE_SET_ADVERTISING_PARAMETERS_CP_SIZE;
    rq.rparam = &status;
    rq.rlen = 1;

    if (hci_send_req(dd, &rq, 500) < 0)
    {
        fprintf(stderr, LOG_ERR_STR "%s: Failed to set advertising parameters, %s\n", name,
            strerror(errno));
        fflush(stderr);
        goto fail;
    }
    le_set_advertise_enable_cp advertise_cp;

    memset(&advertise_cp, 0, sizeof(advertise_cp));
    advertise_cp.enable = 1;

    memset(&rq, 0, sizeof(rq));
    rq.ogf = OGF_LE_CTL;
    rq.ocf = OCF_LE_SET_ADVERTISE_ENABLE;
    rq.cparam = &advertise_cp;
    rq.clen = LE_SET_ADVERTISE_ENABLE_CP_SIZE;
    rq.rparam = &status;
    rq.rlen = 1;

    rc = hci_send_req(dd, &rq, 1000);
fail:
    hci_close_dev(dd);
    return rc;
}

static volatile bool bail = false;
static int dd;
static int fd;

static void term(int signum)
{
    (void)signum;
    bail = true;
}

void* server_thread(void* ptr)
{
    printf(LOG_INFO_STR "BLE server: started\n");
    fflush(stdout);

    sigset_t signal_mask;
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, (int)ptr);
    int rc = pthread_sigmask(SIG_UNBLOCK, &signal_mask, NULL);
    if (rc != 0)
    {
        fprintf(stderr, LOG_ERR_STR "Error setting signal mask\n");
        fflush(stderr);
        return NULL;
    }

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction((int)ptr, &action, NULL);

    while (!bail)
    {
        if (restart_advert() < 0)
        {
            fprintf(stderr, LOG_ERR_STR "%s: Failed to restart advertising\n", name);
            fflush(stderr);
        }
        else
        {
            int dev_id = hci_get_route(NULL);
            dd = hci_open_dev(dev_id);
            if (dd < 0)
            {
                fprintf(stderr, LOG_ERR_STR "%s: Device open failed\n", name);
                fflush(stderr);
                exit(EXIT_FAILURE);
            }

            fd = l2cap_le_att_listen_and_accept();
            if (fd >= 0)
            {
                gatt_service(fd);
                close(fd);
            }
            hci_close_dev(dd);
        }
    }
    close(fd);
    hci_close_dev(dd);
    printf(LOG_INFO_STR "BLE server: stopped\n");
    fflush(stdout);
    return NULL;
}
