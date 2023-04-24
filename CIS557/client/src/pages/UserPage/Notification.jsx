import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import { Typography, Paper, Container } from '@mui/material';

import Layout from '../../components/Layout/Layout';

import ExitNotification from '../../components/UserPage/Notification/ExitNotification';
import MentionNotification from '../../components/UserPage/Notification/MentionNotification';
import DelReqNotification from '../../components/UserPage/Notification/DelReqNotification';
import DelByAdminNotification from '../../components/UserPage/Notification/DelByAdminNotification';
import JoinReqNotification from '../../components/UserPage/Notification/JoinReqNotification';
import AdminNotification from '../../components/UserPage/Notification/AdminNotification';
import InviteNotification from '../../components/UserPage/Notification/InviteNotification';
import InviteResNotification from '../../components/UserPage/Notification/InviteResNotification';
import OwnerNotification from '../../components/UserPage/Notification/OwnerNotification';

import * as fetchUser from '../../utils/fetchUser';

const useStyles = makeStyles({

  content: {
    marginLeft: 'auto',
    marginRight: 'auto',
    marginTop: 20,
    paddingTop: 20,
    paddingBottom: 20,
    width: 800,
  },

  padding: {
    height: 50,
  },
});

export default function Notification() {
  const classes = useStyles();
  const navigate = useNavigate();
  const userID = sessionStorage.getItem('userID');
  const [notifications, setNotifications] = useState(null);

  useEffect(async () => {
    if (!userID) {
      navigate('/login');
    } else {
      const data = await fetchUser.fetchUser(userID);
      if (data === null) {
        navigate('/wrongpage');
      } else {
        setNotifications(data.notifications);
      }
    }
  }, []);

  function renderNotifications() {
    if (notifications) {
      return (
        notifications.map((notification) => {
          switch (notification.type) {
            case 'exit':
              return (<ExitNotification key={notification.id} notification={notification} userID={userID} />);

            case 'mention':
              return (<MentionNotification key={notification.id} notification={notification} userID={userID} />);

            case 'delReq':
              return (<DelReqNotification key={notification.id} notification={notification} userID={userID} />);

            case 'delByAdmin':
              return (<DelByAdminNotification key={notification.id} notification={notification} userID={userID} />);

            case 'joinReq':
              return (<JoinReqNotification key={notification.id} notification={notification} userID={userID} />);

            case 'admin':
              return (<AdminNotification key={notification.id} notification={notification} userID={userID} />);

            case 'invite':
              return (<InviteNotification key={notification.id} notification={notification} userID={userID} />);
            case 'inviteRes':
              return (<InviteResNotification key={notification.id} notification={notification} userID={userID} />);
            case 'owner':
              return (<OwnerNotification key={notification.id} notification={notification} userID={userID} />);
            default:
          }
        })
      );
    }
  }

  return (
    <Layout>
      <Container>

        <Paper className={classes.content} elevation={1}>
          <Typography align="center" variant="h4">
            Notification
          </Typography>
          {
                            renderNotifications()
                        }

        </Paper>
        <div className={classes.padding} />
      </Container>
    </Layout>
  );
}
