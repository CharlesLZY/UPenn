import React, { useState, useEffect } from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import {
  Container, Paper, Typography, IconButton,
} from '@mui/material';

import Home from '@mui/icons-material/Home';

import Layout from '../../components/Layout/Layout';
import RequestBar from '../../components/PrivateGroup/Admin/RequestBar';
import AdminBar from '../../components/PrivateGroup/Admin/AdminBar';
import GroupAnalytics from '../../components/PrivateGroup/Admin/GroupAnalytics';

import * as fetchGroup from '../../utils/fetchPrivateGroup';

const useStyles = makeStyles({
  banner: {
    display: 'flex',
    justifyContent: 'space-between',
    height: 80,
    width: 1170,
  },
  groupName: {
    marginTop: 'auto',
    marginBottom: 'auto',
  },
  homeBtn: {
    height: 50,
    width: 50,
    marginTop: 20,
  },
  bannerPadding: {
    width: 50,
  },

  body: {
    display: 'flex',
    width: 1170,
  },
  padding: {
    height: '50px',
  },
});

export default function Admin() {
  const classes = useStyles();
  const params = useParams();
  const navigate = useNavigate();
  const { groupID } = params;
  const userID = sessionStorage.getItem('userID');

  const [group, setGroup] = useState(null);

  useEffect(async () => {
    if (!userID) {
      navigate('/login');
    } else {
      const data = await fetchGroup.fetchGroup(groupID);
      if (data === null) {
        navigate('/wrongpage');
      } else if (data.members.indexOf(userID) < 0) {
        navigate('/permission');
      } else if (data.admins.indexOf(userID) < 0) {
        navigate(`/privategroup/${groupID}`);
      } else {
        setGroup(data);
      }
    }
  }, []);

  const handleClickHome = () => {
    navigate(`/privategroup/${groupID}`);
  };

  return (
    <Layout>
      <Container>
        <Paper className={classes.banner}>
          <div className={classes.bannerPadding} />
          <div className={classes.groupName}>
            <Typography variant="h3" color="secondary">
              {groupID}
            </Typography>
          </div>
          <div className={classes.homeBtn}>
            <IconButton onClick={handleClickHome}>
              <Home />
            </IconButton>
          </div>
        </Paper>
        <div className={classes.body}>
          {group && <RequestBar group={group} />}
          {group && <AdminBar group={group} userID={userID} />}
          {group && <GroupAnalytics group={group} />}
        </div>

      </Container>
      <div className={classes.padding} />
    </Layout>

  );
}
