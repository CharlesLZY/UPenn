import React, { useState, useEffect } from 'react';
import { makeStyles } from '@mui/styles';
import { Paper, CircularProgress } from '@mui/material';
import { Waypoint } from 'react-waypoint';

import PostCard from './PostCard';

import * as fetchPost from '../../utils/fetchPublicPost';

const useStyles = makeStyles({
  postSection: {
    margin: 25,
  },
  postSectionBG: {
    marginTop: 20,
    paddingTop: 10,
    paddingBottom: 10,
    marginBottom: 20,
    width: 850,
  },
});

export default function PostBar({ postIds }) {
  const classes = useStyles();
  const [posts, setPosts] = useState([]);
  const [loading, setLoading] = useState(false);

  useEffect(async () => {
    // initalization: load first 5 posts
    setPosts(await Promise.all(
      postIds.slice(0, 5).map(
        ({ _id }) => fetchPost.fetchPost(_id),
      ),
    ));
  }, []);

  return (
    <Paper className={classes.postSectionBG}>
      <div className={classes.postSection}>
        {posts && posts.map((post, idx) => (
          <React.Fragment key={post._id}>
            <Waypoint onEnter={async () => {
              if (idx === posts.length - 1 && postIds.length > posts.length) {
                setLoading(true);
                const newPost = await fetchPost.fetchPost(postIds[posts.length]._id);
                // eslint-disable-next-line no-promise-executor-return
                await new Promise((r) => setTimeout(r, 300)); // exaggerated effect
                setPosts((oldPosts) => [...oldPosts, newPost]);
                setLoading(false);
              }
            }}
            />
            <PostCard post={post} />
          </React.Fragment>
        ))}
        {/* brute force centering */}
        { loading && <CircularProgress sx={{ marginLeft: '375px' }} /> }
      </div>
    </Paper>
  );
}
