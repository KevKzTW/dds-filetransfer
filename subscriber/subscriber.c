#include <fcntl.h>
#include "FTP.h"

#define MAX_SAMPLES 10

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

    dds_entity_t subscriber;
    error = dds_subscriber_create(participant, &subscriber, qos_pubsub, NULL);
    DDS_ERR_CHECK(error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    dds_qos_delete(qos_pubsub);

    dds_qos_t *qos = dds_qos_create();
    dds_qset_durability(qos, DDS_DURABILITY_TRANSIENT_LOCAL);
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(10));
    dds_qset_history(qos, DDS_HISTORY_KEEP_LAST, MAX_SAMPLES);

    dds_entity_t reader;
    error = dds_reader_create(subscriber, &reader, topic, qos, NULL);
    DDS_ERR_CHECK(error, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    dds_qos_delete(qos);

    FILE *fd_to = NULL;
    bool is_done = false;
    dds_sample_info_t info[MAX_SAMPLES] = {{0}};
    void *samples[MAX_SAMPLES] = {0};
    uint32_t mask = DDS_NOT_READ_SAMPLE_STATE | DDS_ANY_VIEW_STATE | DDS_ALIVE_INSTANCE_STATE;
    do
    {
        int read_count = dds_take(reader, samples, MAX_SAMPLES, info, mask);
        for (int i = 0; i < read_count; i++)
        {
            if (info[i].valid_data)
            {
                FTP_File *sample_ptr = samples[i];

                ssize_t nread = sample_ptr->payload._length;

                printf("Recv seq %d with %d bytes, is done %d\n", sample_ptr->seq_num, nread, sample_ptr->is_done);
                if (sample_ptr->is_done)
                {
                    is_done = true;
                    break;
                }

                if (fd_to == NULL)
                {
                    char *to_path = sample_ptr->path;
                    fd_to = open(to_path, O_WRONLY | O_CREAT | O_EXCL, 0666);
                    printf("Open %s\n", to_path);
                }

                char *buf = sample_ptr->payload._buffer;

                write(fd_to, buf, nread);
            }
        }
        dds_return_loan(reader, samples, read_count);

        dds_sleepfor(DDS_SECS(1));
    } while (!is_done);

    close(fd_to);

    dds_entity_delete(subscriber);
    dds_entity_delete(participant);
    dds_fini();

    return 0;
}
