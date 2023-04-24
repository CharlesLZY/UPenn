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

  link: {
    '&:hover': {
      color: '#F5AF64',
      cursor: 'pointer',
    },
  },

});

export default function MentionNotification({ notification, userID }) {
  const classes = useStyles();
  const navigate = useNavigate();

  const handleClickClose = async () => {
    await fetchUser.deleteNotification(userID, notification.id);
    navigate(0);
  };

  const handleClickLink = async (link) => {
    navigate(link);
  };

  return (
    <Paper className={classes.card}>
      <IconButton sx={{ marginLeft: '660px' }} onClick={handleClickClose}>
        <Close />
      </IconButton>
      <div className={classes.content}>
        <Typography variant="h5" color="secondary">
          {notification.from}
        </Typography>
        <Typography variant="h5">
                    &thinsp; mentioned &thinsp;you &thinsp;in &thinsp;
        </Typography>
        <Typography variant="h5" className={classes.link} color="secondary" onClick={() => handleClickLink(notification.to)}>
          {notification.status}
        </Typography>
        <Typography variant="h5">
          .
        </Typography>
      </div>
    </Paper>

  );
}
