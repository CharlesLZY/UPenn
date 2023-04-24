import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import {
  Container, Paper, Typography, Avatar,
} from '@mui/material';

import Layout from '../../components/Layout/Layout';
import SecurityBtn from '../../components/UserPage/SecurityBtn';
import PostBar from '../../components/UserPage/PostBar';
import GroupBar from '../../components/UserPage/GroupBar';

import * as fetchUser from '../../utils/fetchUser';
import * as utils from '../../utils/utils';

const useStyles = makeStyles({
  banner: {
    display: 'inlined',
    height: 95,
    width: 1170,
  },

  avatar: {
    marginTop: 'auto',
    marginBottom: 'auto',
    marginLeft: 20,
    marginRight: 20,
  },

  left: {
    display: 'flex',
    marginTop: 10,
  },

  date: {
    marginLeft: 5,
  },
  info: {
    display: 'flex',
    justifyContent: 'space-between',
    marginTop: 10,
  },

  body: {
    display: 'flex',
    width: 1170,
  },

});

export default function UserPage() {
  const classes = useStyles();
  const navigate = useNavigate();
  const userID = sessionStorage.getItem('userID');
  const [userInfo, setUserInfo] = useState(null);
  const [posts, setPosts] = useState(null);

  useEffect(async () => {
    if (!userID) {
      navigate('/login');
    } else {
      const data = await fetchUser.fetchUser(userID);
      if (data === null) {
        navigate('/wrongpage');
      } else {
        setUserInfo(data);
        const res = await fetchUser.fetchUserPost(userID);
        setPosts(res);
      }
    }
  }, []);

  return (
    <Layout>
      <Container>
        <Paper className={classes.banner}>
          <div className={classes.info}>

            <div className={classes.left}>
              <Avatar className={classes.avatar} sx={{ width: '60px', height: '60px' }} />
              <div>
                <Typography variant="h3" color="secondary">
                  {userID}
                </Typography>

                <Typography className={classes.date} variant="subtitle2">
                  {userInfo && `Created on ${utils.convertTime(userInfo.register_date)}`}
                </Typography>
              </div>

            </div>
            <SecurityBtn />
          </div>

        </Paper>
        <div className={classes.body}>
          {posts && <PostBar posts={posts} />}
          <GroupBar userInfo={userInfo} />
        </div>
      </Container>
    </Layout>
  );
}
