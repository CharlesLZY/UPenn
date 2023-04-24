import React from 'react';
import { useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import {
  Typography, Paper, IconButton, Button,
} from '@mui/material';

import Close from '@mui/icons-material/Close';
import Check from '@mui/icons-material/Check';

import * as fetchPublicGroup from '../../../utils/fetchPublicGroup';
import * as fetchPrivateGroup from '../../../utils/fetchPrivateGroup';
import * as fetchUser from '../../../utils/fetchUser';

const useStyles = makeStyles({
  card: {
    width: 700,
    height: 140,
    marginLeft: 'auto',
    marginRight: 'auto',
    marginTop: 10,
  },

  content: {
    display: 'flex',
    justifyContent: 'center',
  },

  btnArea: {
    display: 'flex',
    justifyContent: 'center',
  },

});

export default function InviteNotification({ notification, userID }) {
  const classes = useStyles();
  const navigate = useNavigate();

  const handleClickClose = async () => {
    await fetchUser.deleteNotification(userID, notification.id);
    navigate(0);
  };

  const handleClickYes = async () => {
    if (notification.status === 'public') {
      await fetchPublicGroup.requestJoin(notification.to, userID);
    } else if (notification.status === 'private') {
      await fetchPrivateGroup.requestJoin(notification.to, userID);
    }

    await fetchUser.acceptInvitation(userID, notification.from, notification.to);
    await fetchUser.deleteNotification(userID, notification.id);
    navigate(0);
  };

  const handleClickNo = async () => {
    await fetchUser.rejectInvitation(userID, notification.from, notification.to);
    await fetchUser.deleteNotification(userID, notification.id);
    navigate(0);
  };

  return (
    <Paper className={classes.card}>
      <IconButton sx={{ marginLeft: '660px' }} onClick={handleClickClose}>
        <Close />
      </IconButton>
      <div className={classes.content}>
        <Typography variant="h6" color="secondary">
          {notification.from}
        </Typography>
        <Typography variant="h6">
                    &thinsp; invited you to join &thinsp;
        </Typography>
        <Typography variant="h6" color="secondary">
          {notification.to}
          {' '}
          (
          {notification.status}
          )
        </Typography>
        <Typography variant="h6">
          .
        </Typography>
      </div>
      <div className={classes.btnArea}>
        <Button
          startIcon={<Check />}
          sx={{
            fontSize: 20, marginTop: '5px', marginLeft: '50px', marginRight: '50px', color: '#000000',
          }}
          onClick={handleClickYes}
        >
          Yes
        </Button>
        <Button
          startIcon={<Close />}
          sx={{
            fontSize: 20, marginTop: '5px', marginLeft: '50px', marginRight: '50px', color: '#000000',
          }}
          onClick={handleClickNo}
        >
          No
        </Button>
      </div>
    </Paper>

  );
}
