import React, { useEffect, useState } from 'react';

import { makeStyles } from '@mui/styles';
import { Container } from '@mui/material';

import Layout from '../components/Layout/Layout';
import PostBar from '../components/Home/PostBar';
import GroupBar from '../components/Home/GroupBar';
import SuggestedGroup from '../components/Home/SuggestedGroup';

import * as fetchPost from '../utils/fetchPublicPost';
import * as fetchGroup from '../utils/fetchPublicGroup';
import * as fetchUser from '../utils/fetchUser';

const useStyles = makeStyles({
  right: {
    position: 'fixed',
    marginLeft: 845,
  },
});

export default function Home() {
  const classes = useStyles();
  const [postIds, setPostIds] = useState(null);
  const [groups, setGroups] = useState(null);
  const [suggestedGroup, setSuggestedGroup] = useState(null);
  const [userInfo, setUserInfo] = useState(null);
  const userID = sessionStorage.getItem('userID');

  useEffect(async () => {
    setPostIds(await fetchPost.fetchPopularPostId());
    setGroups(await fetchGroup.fetchAllPublicGroups());
    if (userID) {
      setSuggestedGroup(await fetchGroup.fetchSuggestedGroup(userID));
      setUserInfo(await fetchUser.fetchUser(userID));
    }
  }, []);

  return (
    <Layout>
      <Container sx={{ display: 'flex' }}>
        {postIds && <PostBar postIds={postIds} />}
        <div className={classes.right}>
          {groups && <GroupBar groups={groups} userInfo={userInfo} />}
          {userID && <SuggestedGroup suggestion={suggestedGroup} />}
        </div>
      </Container>
    </Layout>

  );
}
