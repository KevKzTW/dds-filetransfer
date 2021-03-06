#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#include <fcntl.h>
#include "FTP.h"

int main(int argc, char **argv)
{
    dds_init(argc, argv);

    dds_entity_t participant;
    int error = dds_participant_create(&participant, DDS_DOMAIN_DEFAULT, NULL, NULL);
    DDS_ERR_CHECK(error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    dds_entity_t topic;
    error = dds_topic_create(participant, &topic, &FTP_File_desc, "FTP", NULL, NULL);
    DDS_ERR_CHECK(error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    dds_qos_t *qos_pubsub = dds_qos_create();
    const char *partitions = "Test";
    dds_qset_partition(qos_pubsub, 1, &partitions);

    dds_entity_t publisher;
    error = dds_publisher_create(participant, &publisher, qos_pubsub, NULL);
    DDS_ERR_CHECK(error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    dds_qos_delete(qos_pubsub);

    dds_qos_t *qos = dds_qos_create();
    dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));

    dds_entity_t writer;
    error = dds_writer_create(publisher, &writer, topic, qos, NULL);
    DDS_ERR_CHECK(error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    dds_qos_delete(qos);

    char *from_path = "test_binary";
    char *to_path = "/root/Desktop/Sended_By_DDS";

    int fd_from = open(from_path, O_RDONLY);
    if (fd_from < 0)
        return -1;

    char buf[32];
    ssize_t nread;
    int i = 0;
    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        FTP_File sample = {0};
        sample.path = to_path;
        sample.seq_num = i;
        sample.payload._buffer = buf;
        sample.payload._length = nread;
        sample.payload._release = true;
        sample.is_done = false;

        error = dds_write(writer, &sample);
        DDS_ERR_CHECK(error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

        i++;

        dds_sleepfor(DDS_SECS(1));
    }

    close(fd_from);

    FTP_File sample = {0};
    sample.path = to_path;
    sample.seq_num = i;
    sample.payload._buffer = NULL;
    sample.payload._length = 0;
    sample.payload._release = false;
    sample.is_done = true;

    error = dds_write(writer, &sample);
    DDS_ERR_CHECK(error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    dds_sleepfor(DDS_SECS(1));

    dds_entity_delete(publisher);
    dds_entity_delete(participant);
    dds_fini();

    return 0;
}
