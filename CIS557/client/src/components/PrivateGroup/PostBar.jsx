import React, { useState } from 'react';
import { makeStyles } from '@mui/styles';
import { Paper, TextField, IconButton } from '@mui/material';

import Close from '@mui/icons-material/Close';
import FilterListIcon from '@mui/icons-material/FilterList';

import PostCard from './PostCard';

const useStyles = makeStyles({

  postSection: {
    margin: 25,
  },

  postSectionBG: {
    marginTop: 10,
    paddingTop: 10,
    paddingBottom: 10,
    width: 850,
  },

  search: {
    marginLeft: 25,
  },

});

export default function PostBar({ posts, userInfo, group }) {
  const classes = useStyles();

  const [input, setInput] = useState('');
  const [postsToShow, setPostsToShow] = useState(posts);

  const handleReset = () => {
    document.getElementById('hashtag-inputBar').value = '';
    setInput('');
    setPostsToShow(posts);
  };

  const handleFilter = () => {
    setPostsToShow(posts.filter((post) => post.hashtags.indexOf(input) >= 0));
  };

  function renderPosts(postsToShow) {
    return (
      <Paper className={classes.postSectionBG}>
        <div className={classes.search}>
          <TextField
            size="small"
            color="secondary"
            label="Hashtag"
            id="hashtag-inputBar"
            inputProps={{ maxLength: 30 }}
            onChange={(e) => setInput(e.target.value)}
          />
          <IconButton onClick={handleReset}>
            <Close />
          </IconButton>
          <IconButton onClick={handleFilter}>
            <FilterListIcon />
          </IconButton>
        </div>
        <div className={classes.postSection}>
          {postsToShow.map((post) => (
            <PostCard key={post._id} post={post} userInfo={userInfo} group={group} />
          ))}
        </div>
      </Paper>
    );
  }

  return (
    renderPosts(postsToShow)
  );
}
