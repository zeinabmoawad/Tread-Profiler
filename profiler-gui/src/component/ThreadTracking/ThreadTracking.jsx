import React, { useEffect, useState } from 'react';
import { Table, Input, Select, Button, Space, Popconfirm, message, Typography, notification } from 'antd';
import { DeleteOutlined } from '@ant-design/icons'; 
import './ThreadTracking.css'; 

const { Search } = Input;
const { Option } = Select;

const ThreadTracking = () => {
  const [threadMetrics, setThreadMetrics] = useState([]);
  const [filteredMetrics, setFilteredMetrics] = useState([]);
  const [searchText, setSearchText] = useState('');
  const [selectedState, setSelectedState] = useState('');

  // Handle delete action
  const handleDelete = (threadId) => {
    setThreadMetrics((prevMetrics) =>
      prevMetrics.filter((metric) => metric.threadId !== threadId)
    );
    message.success('Thread data deleted successfully.');
  };

//  Web socket
  useEffect(() => {
    const socket = new WebSocket('ws://localhost:8080');
    let buffer = '';

    socket.onopen = () => {
      console.log('Connected to WebSocket server.');
    };
    socket.onmessage = (event) => {
      buffer += event.data;
      let boundary = buffer.lastIndexOf('\n');
      
      const messages = buffer.substring(0, boundary).split('\n');
      buffer = buffer.substring(boundary + 1); // Retain incomplete data

      messages.forEach((messageSent) => {
        if (messageSent.trim()) {

          console.log(`parsedData ${messageSent}`)
          let parsedData;

          try {
            parsedData = JSON.parse(messageSent);
          } catch (parseError) {
            console.error('Error parsing JSON', parseError);
            return;
          }
          if (parsedData.type === 'deadlock') {
            notification.error({
              message: 'Deadlock',
              description: `Deadlock detected in your code.`,
              duration: 0,
              placement: 'top',
            });

            return;
          }

          // Ensure parsedData is in array form for consistency
          if (!Array.isArray(parsedData)) {
            parsedData = [parsedData];
          }

          // Update or add new metric based on threadId
          setThreadMetrics((prevMetrics) => {
            const updatedMetrics = [...prevMetrics];

            parsedData.forEach((data) => {
              const existingMetricIndex = updatedMetrics.findIndex(
                (metric) => metric.threadId === data.threadId
              );

              if (existingMetricIndex !== -1) {
                // Update existing metric
                updatedMetrics[existingMetricIndex] = {
                  ...updatedMetrics[existingMetricIndex],
                  ...data, // Merge updated data
                };
              } else {
                // Add new metric if not found
                updatedMetrics.push(data);
              }
            });

            return updatedMetrics;
          });
        }
      });
    };
    socket.onerror = (error) => {
      console.error('WebSocket error:', error);
    };
    socket.onclose = () => {
      console.log('WebSocket connection closed');
    };

    return () => {
      socket.close();
    };
  }, []);

  // Update filteredMetrics when threadMetrics, searchText, or selectedState changes
  useEffect(() => {
    const lowerSearchText = searchText.toLowerCase();
    const filtered = threadMetrics.filter(
      (metric) =>
        (metric.threadName.toLowerCase().includes(lowerSearchText) ||
          metric.threadId.toString().includes(lowerSearchText)) &&
        (selectedState ? metric.state === selectedState : true)
    );
    setFilteredMetrics(filtered);
  }, [threadMetrics, searchText, selectedState]);

  // Define table columns
  const columns = [
    {
      title: 'Thread Name',
      dataIndex: 'threadName',
      key: 'threadName',
    },
    {
      title: 'Thread ID',
      dataIndex: 'threadId',
      key: 'threadId',
    },
    {
      title: 'Start Time (seconds)',
      dataIndex: 'startTime',
      key: 'startTime',
    },
    {
      title: 'End Time (seconds)',
      dataIndex: 'endTime',
      key: 'endTime',
    },
    {
      title: 'State',
      dataIndex: 'state',
      key: 'state',
      render: (text) => {
        const className =
          text === 'Running'
            ? 'state-ready'
            : text === 'Waiting'
              ? 'state-wait'
              : text === 'Finished'
                ? 'state-finish'
                : '';
        return <span className={className}>{text}</span>;
      },
    },
    {
      title: 'Mutex Acquired',
      dataIndex: 'mutexAcquired',
      key: 'mutexAcquired',
    },
    {
      title: 'Waiting Time',
      dataIndex: 'waitingTime',
      key: 'waitingTime',
    },
    {
      title: '',
      key: 'actions',
      render: (text, record) => (
        <Space size="middle">
          <Popconfirm
            title="Are you sure you want to delete this thread?"
            onConfirm={() => handleDelete(record.threadId)}
            okText="Yes"
            cancelText="No"
          >
            <Button type="text" icon={<DeleteOutlined />} danger ghost>
            </Button>
          </Popconfirm>
        </Space>
      ),
    },
  ];

  return (
    <div style={{margin:"1%"}}>
      <div >
        <Typography.Title level={1} style={{color:"#1677ff"}}>Thread Tracker</Typography.Title>
        
        </div>
      <div style={{ marginBottom: 16, display: 'flex', gap: '10px' }}>
        <Search
          placeholder="Search by Thread Name or ID"
          enterButton
          onSearch={(value) => setSearchText(value)} 
          style={{ width: 300 }} 
        />
        <Select
          placeholder="Filter by State"
          onChange={(value) => setSelectedState(value)}
          allowClear
          style={{ width: 200 }}
        >
          <Option value="Running">Running</Option>
          <Option value="Waiting">Waiting</Option>
          <Option value="Finished">Finished</Option>
        </Select>
      </div>
      <Table
        className="thread-metrics-table"
        dataSource={filteredMetrics}
        columns={columns}
        rowKey="threadId"
        pagination={{ pageSize: 20 }}
        rowClassName={(record) =>
          record.state === 'Running'
            ? 'state-ready'
            : record.state === 'Waiting'
              ? 'state-wait'
              : record.state === 'Finished'
                ? 'state-finish'
                : ''
        }
      />

    </div>
  );
};

export default ThreadTracking;
