import React from 'react';
import { useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import { Typography, Paper, IconButton } from '@mui/material';

import Close from '@mui/icons-material/Close';
import * as fetchUser from '../../../utils/fetchUser';

const useStyles = makeStyles({
  card: {
    width: 700,
    height: 100,
    marginLeft: 'auto',
    marginRight: 'auto',
    marginTop: 10,
  },

  content: {
    display: 'flex',
    justifyContent: 'center',
  },

});

export default function AdminNotification({ notification, userID }) {
  const classes = useStyles();
  const navigate = useNavigate();

  const handleClickClose = async () => {
    await fetchUser.deleteNotification(userID, notification.id);
    navigate(0);
  };
  if (notification.status === 'promoted') {
    return (
      <Paper className={classes.card}>
        <IconButton sx={{ marginLeft: '660px' }} onClick={handleClickClose}>
          <Close />
        </IconButton>
        <div className={classes.content}>
          <Typography variant="h6">
            You are promoted as admin of &thinsp;
          </Typography>
          <Typography variant="h6" color="secondary">
            {notification.from}
          </Typography>
          <Typography variant="h6">
            .
          </Typography>
        </div>
      </Paper>
    );
  }

  return (
    <Paper className={classes.card}>
      <IconButton sx={{ marginLeft: '660px' }} onClick={handleClickClose}>
        <Close />
      </IconButton>
      <div className={classes.content}>
        <Typography variant="h6">
          Your admin status of &thinsp;
        </Typography>
        <Typography variant="h6" color="secondary">
          {notification.from}
          {' '}
&thinsp;
        </Typography>
        <Typography variant="h6">
          was revoked.
        </Typography>
      </div>
    </Paper>
  );
}
